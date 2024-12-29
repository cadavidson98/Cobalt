#ifndef COBALT_CLI_PARSER_H
#define COBALT_CLI_PARSER_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace cblt::cli {

std::optional<CoCLIParams> parseArguments(int numArgs, char **argv);
} // namespace cblt::cli

#endif // COBALT_CLI_PARSER_H
