#include "network/tcp_server.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <stdexcept>
#include <unordered_map>

namespace rediss {

static constexpr int MAX_EVENTS = 64;
static constexpr int READ_BUFFER_SIZE = 4096;
static constexpr int LISTEN_BACKLOG = 128;

TcpServer::TcpServer(int port, RequestHandler handler)
    : port_(port), handler_(std::move(handler)) {}

TcpServer::~TcpServer() {
    // Close all client connections
    for (auto& [fd, _] : client_buffers_) {
        close(fd);
    }
    if (listener_fd_ >= 0) close(listener_fd_);
    if (epoll_fd_ >= 0) close(epoll_fd_);
}

void TcpServer::start() {
    createListenerSocket();

    // Create epoll instance
    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ < 0) {
        throw std::runtime_error("epoll_create1 failed: " + std::string(strerror(errno)));
    }

    // Register the listener socket with epoll
    struct epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = listener_fd_;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, listener_fd_, &ev) < 0) {
        throw std::runtime_error("epoll_ctl ADD listener failed");
    }

    std::cout << "Rediss server listening on port " << port_ << std::endl;

    // Main event loop
    struct epoll_event events[MAX_EVENTS];

    while (true) {
        int num_events = epoll_wait(epoll_fd_, events, MAX_EVENTS, -1);
        if (num_events < 0) {
            if (errno == EINTR) continue;  // Interrupted by signal, retry
            throw std::runtime_error("epoll_wait failed: " + std::string(strerror(errno)));
        }

        for (int i = 0; i < num_events; ++i) {
            int fd = events[i].data.fd;

            if (fd == listener_fd_) {
                // New incoming connection
                acceptNewClient();
            } else {
                // Data available on an existing client socket
                if (events[i].events & (EPOLLERR | EPOLLHUP)) {
                    removeClient(fd);
                } else if (events[i].events & EPOLLIN) {
                    handleClientData(fd);
                }
            }
        }
    }
}

void TcpServer::createListenerSocket() {
    // Create TCP socket
    listener_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listener_fd_ < 0) {
        throw std::runtime_error("socket() failed: " + std::string(strerror(errno)));
    }

    // Allow port reuse (avoids "Address already in use" on restarts)
    int opt = 1;
    if (setsockopt(listener_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");
    }

    // Make the listener non-blocking
    setNonBlocking(listener_fd_);

    // Bind to 0.0.0.0:<port>
    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(static_cast<uint16_t>(port_));

    if (bind(listener_fd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        throw std::runtime_error("bind() failed on port " + std::to_string(port_) +
                                 ": " + std::string(strerror(errno)));
    }

    if (listen(listener_fd_, LISTEN_BACKLOG) < 0) {
        throw std::runtime_error("listen() failed: " + std::string(strerror(errno)));
    }
}

void TcpServer::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        throw std::runtime_error("fcntl(F_GETFL) failed");
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        throw std::runtime_error("fcntl(F_SETFL) failed");
    }
}

void TcpServer::acceptNewClient() {
    // Accept all pending connections (edge-triggered style)
    while (true) {
        struct sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(listener_fd_,
                               reinterpret_cast<struct sockaddr*>(&client_addr),
                               &client_len);
        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;  // No more pending connections
            }
            std::cerr << "accept() failed: " << strerror(errno) << std::endl;
            return;
        }

        setNonBlocking(client_fd);

        // Register the new client with epoll
        struct epoll_event ev{};
        ev.events = EPOLLIN;
        ev.data.fd = client_fd;
        if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, client_fd, &ev) < 0) {
            std::cerr << "epoll_ctl ADD client failed" << std::endl;
            close(client_fd);
            return;
        }

        // Initialize an empty read buffer for this client
        client_buffers_[client_fd] = "";

        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str));
        std::cout << "Client connected: " << ip_str
                  << ":" << ntohs(client_addr.sin_port)
                  << " (fd=" << client_fd << ")" << std::endl;
    }
}

void TcpServer::handleClientData(int client_fd) {
    char buf[READ_BUFFER_SIZE];

    while (true) {
        ssize_t bytes_read = read(client_fd, buf, sizeof(buf));

        if (bytes_read < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;  // No more data available right now
            }
            // Read error — drop the client
            removeClient(client_fd);
            return;
        }

        if (bytes_read == 0) {
            // Client disconnected gracefully
            removeClient(client_fd);
            return;
        }

        // Append received data to this client's buffer
        client_buffers_[client_fd].append(buf, static_cast<size_t>(bytes_read));
    }

    // Process all complete lines (delimited by \n) in the buffer
    auto& buffer = client_buffers_[client_fd];
    size_t pos;

    while ((pos = buffer.find('\n')) != std::string::npos) {
        // Extract one complete line (including the \n)
        std::string line = buffer.substr(0, pos + 1);
        buffer.erase(0, pos + 1);

        // Pass to handler and write response
        std::string response = handler_(line);

        if (!response.empty()) {
            // Write the full response (for simplicity, blocking write)
            // In production, you'd buffer writes and use EPOLLOUT.
            ssize_t total_written = 0;
            ssize_t to_write = static_cast<ssize_t>(response.size());

            while (total_written < to_write) {
                ssize_t written = write(client_fd,
                                        response.c_str() + total_written,
                                        static_cast<size_t>(to_write - total_written));
                if (written < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        // Could not write immediately — in a real server we'd
                        // buffer and register for EPOLLOUT. For now, retry.
                        continue;
                    }
                    removeClient(client_fd);
                    return;
                }
                total_written += written;
            }
        }
    }
}

void TcpServer::removeClient(int client_fd) {
    std::cout << "Client disconnected (fd=" << client_fd << ")" << std::endl;
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, client_fd, nullptr);
    close(client_fd);
    client_buffers_.erase(client_fd);
}

} // namespace rediss
