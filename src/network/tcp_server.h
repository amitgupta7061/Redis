#pragma once

#include <functional>
#include <string>
#include <unordered_map>

namespace rediss {

/// Non-blocking TCP server using Linux epoll.
///
/// The server accepts connections on the specified port and reads commands
/// line-by-line from each client. Each complete line is passed to the
/// user-provided callback, and the callback's return value is written back
/// to the client.
///
/// Design notes:
///   - All sockets are set non-blocking
///   - epoll is used for I/O multiplexing (single-threaded event loop)
///   - Per-client read buffers accumulate partial reads until a newline
///   - Graceful handling of client disconnects and errors
class TcpServer {
public:
    /// Callback type: takes a raw command string, returns command response.
    using RequestHandler = std::function<std::string(const std::string&)>;

    /// Construct a server on the given port with a request handler callback.
    TcpServer(int port, RequestHandler handler);

    /// Destructor — closes the listener and epoll file descriptors.
    ~TcpServer();

    // Non-copyable
    TcpServer(const TcpServer&) = delete;
    TcpServer& operator=(const TcpServer&) = delete;

    /// Start the event loop (blocking).
    /// Creates the socket, binds, listens, and enters epoll_wait loop.
    void start();

private:
    void createListenerSocket();
    void setNonBlocking(int fd);
    void acceptNewClient();
    void handleClientData(int client_fd);
    void removeClient(int client_fd);

    int port_;
    int listener_fd_ = -1;
    int epoll_fd_ = -1;
    RequestHandler handler_;

    // Per-client read buffers to handle partial reads
    std::unordered_map<int, std::string> client_buffers_;
};

} // namespace rediss
