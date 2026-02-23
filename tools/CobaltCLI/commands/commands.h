#ifndef COBALT_CLI_COMMANDS_H
#define COBALT_CLI_COMMANDS_H

#include <span>

namespace cobalt::cli {

bool renderCommand(std::span<char *> args);

bool preprocessCommand(std::span<char *> args);

} // namespace cobalt::cli

#endif // COBALT_CLI_COMMANDS_H
