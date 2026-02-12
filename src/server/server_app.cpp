#include "server/server_app.h"

#include "commands/command_handler.h"
#include "network/tcp_server.h"
#include "parser/command_parser.h"
#include "store/kv_store.h"

namespace rediss {

void ServerApp::run(int port) {
    // Create the core components
    KVStore store;
    CommandHandler handler(store);

    // Create the TCP server with a lambda that wires the full pipeline:
    //   raw_input → CommandParser::parse → CommandHandler::execute → response
    TcpServer server(port, [&handler](const std::string& raw_input) -> std::string {
        Command cmd = CommandParser::parse(raw_input);
        return handler.execute(cmd);
    });

    server.start();
}

} // namespace rediss
