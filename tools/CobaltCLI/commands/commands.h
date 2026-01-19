#ifndef CBLT_CLI_COMMANDS_H
#define CBLT_CLI_COMMANDS_H

#include <span>

namespace cblt::cli {

bool renderCommand(std::span<char *> args);

bool preprocessCommand(std::span<char *> args);

} // namespace cblt::cli

#endif // CBLT_CLI_COMMANDS_H
