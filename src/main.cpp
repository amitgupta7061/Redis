#include <iostream>
#include <stdexcept>

#include "server/server_app.h"

static constexpr int DEFAULT_PORT = 6379;

int main(int argc, char* argv[]) {
    int port = DEFAULT_PORT;

    // Optional: allow port override via command-line argument
    if (argc >= 2) {
        try {
            port = std::stoi(argv[1]);
        } catch (const std::exception&) {
            std::cerr << "Usage: " << argv[0] << " [port]" << std::endl;
            return 1;
        }
    }

    std::cout << R"(
  ____          _ _
 |  _ \ ___  __| (_)___ ___
 | |_) / _ \/ _` | / __/ __|
 |  _ <  __/ (_| | \__ \__ \
 |_| \_\___|\__,_|_|___/___/

)" << std::endl;

    try {
        rediss::ServerApp app;
        app.run(port);
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
