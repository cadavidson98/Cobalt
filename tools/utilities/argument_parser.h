#ifndef COBALT_CLI_PARSER_H
#define COBALT_CLI_PARSER_H

#include "core/size_types.h"

#include <optional>
#include <string>
#include <vector>

namespace cobalt::cli {

std::optional<CoCLIParams> parseArguments(int numArgs, char **argv);
} // namespace cobalt::cli

#endif // COBALT_CLI_PARSER_H
