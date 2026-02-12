#include "commands/command_handler.h"

namespace rediss {

CommandHandler::CommandHandler(KVStore& store) : store_(store) {}

std::string CommandHandler::execute(const Command& cmd) {
    if (cmd.name.empty()) {
        return "";  // Empty input, no response
    }

    if (cmd.name == "SET")    return handleSet(cmd);
    if (cmd.name == "GET")    return handleGet(cmd);
    if (cmd.name == "DEL")    return handleDel(cmd);
    if (cmd.name == "EXISTS") return handleExists(cmd);
    if (cmd.name == "PING")   return handlePing(cmd);

    return "-ERR unknown command '" + cmd.name + "'\r\n";
}

// SET key value
// Response: +OK\r\n
std::string CommandHandler::handleSet(const Command& cmd) {
    if (cmd.args.size() != 2) {
        return "-ERR wrong number of arguments for 'SET' command\r\n";
    }
    store_.set(cmd.args[0], cmd.args[1]);
    return "+OK\r\n";
}

// GET key
// Response: $<len>\r\n<value>\r\n  or  $-1\r\n (nil)
std::string CommandHandler::handleGet(const Command& cmd) {
    if (cmd.args.size() != 1) {
        return "-ERR wrong number of arguments for 'GET' command\r\n";
    }
    auto value = store_.get(cmd.args[0]);
    if (value.has_value()) {
        return "$" + std::to_string(value->size()) + "\r\n" + *value + "\r\n";
    }
    return "$-1\r\n";
}

// DEL key [key ...]
// Response: :<count>\r\n (number of keys deleted)
std::string CommandHandler::handleDel(const Command& cmd) {
    if (cmd.args.empty()) {
        return "-ERR wrong number of arguments for 'DEL' command\r\n";
    }
    int count = 0;
    for (const auto& key : cmd.args) {
        if (store_.del(key)) {
            ++count;
        }
    }
    return ":" + std::to_string(count) + "\r\n";
}

// EXISTS key
// Response: :1\r\n or :0\r\n
std::string CommandHandler::handleExists(const Command& cmd) {
    if (cmd.args.size() != 1) {
        return "-ERR wrong number of arguments for 'EXISTS' command\r\n";
    }
    return store_.exists(cmd.args[0]) ? ":1\r\n" : ":0\r\n";
}

// PING [message]
// Response: +PONG\r\n or $<len>\r\n<message>\r\n
std::string CommandHandler::handlePing(const Command& cmd) {
    if (cmd.args.empty()) {
        return "+PONG\r\n";
    }
    return "$" + std::to_string(cmd.args[0].size()) + "\r\n" + cmd.args[0] + "\r\n";
}

} // namespace rediss
