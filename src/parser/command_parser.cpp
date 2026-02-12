#include "parser/command_parser.h"

#include <algorithm>
#include <sstream>

namespace rediss {

Command CommandParser::parse(const std::string& raw_input) {
    Command cmd;

    // Strip \r and \n from input (clients may send "SET key val\r\n")
    std::string input = raw_input;
    input.erase(std::remove(input.begin(), input.end(), '\r'), input.end());
    input.erase(std::remove(input.begin(), input.end(), '\n'), input.end());

    // Tokenize on whitespace
    std::istringstream stream(input);
    std::string token;

    // First token is the command name — uppercase it for case-insensitive matching
    if (stream >> token) {
        std::transform(token.begin(), token.end(), token.begin(), ::toupper);
        cmd.name = std::move(token);
    }

    // Remaining tokens are arguments
    while (stream >> token) {
        cmd.args.push_back(std::move(token));
    }

    return cmd;
}

} // namespace rediss
