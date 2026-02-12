#pragma once

#include <string>
#include "commands/command.h"
#include "store/kv_store.h"

namespace rediss {

/// Routes parsed commands to the KVStore and formats Redis-compatible responses.
///
/// Response format follows the Redis Serialization Protocol (RESP):
///   +OK\r\n              — Simple string (success)
///   -ERR message\r\n     — Error
///   :N\r\n               — Integer
///   $N\r\ndata\r\n       — Bulk string
///   $-1\r\n              — Null bulk string (key not found)
class CommandHandler {
public:
    /// Constructor takes a reference to the shared KVStore.
    explicit CommandHandler(KVStore& store);

    /// Execute a parsed command and return the RESP-formatted response.
    std::string execute(const Command& cmd);

private:
    std::string handleSet(const Command& cmd);
    std::string handleGet(const Command& cmd);
    std::string handleDel(const Command& cmd);
    std::string handleExists(const Command& cmd);
    std::string handlePing(const Command& cmd);

    KVStore& store_;
};

} // namespace rediss
