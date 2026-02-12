#pragma once

#include <string>
#include "commands/command.h"

namespace rediss {

/// Parses raw client input into a structured Command.
///
/// The parser is stateless — it operates purely on the input string.
/// It splits on whitespace, uppercases the verb, and returns a Command struct.
class CommandParser {
public:
    /// Parse a raw input line like "SET key value\r\n" into a Command.
    /// Leading/trailing whitespace and \r\n are stripped.
    static Command parse(const std::string& raw_input);
};

} // namespace rediss
