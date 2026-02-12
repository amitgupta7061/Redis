#pragma once

#include <string>
#include <vector>

namespace rediss {

/// Structured representation of a parsed client command.
///
/// Example: "SET mykey myvalue" becomes:
///   Command { name = "SET", args = {"mykey", "myvalue"} }
struct Command {
    std::string name;                // Uppercased command verb (SET, GET, DEL, EXISTS)
    std::vector<std::string> args;   // Positional arguments following the verb
};

} // namespace rediss
