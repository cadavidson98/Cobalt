#include "commands.h"

#include <cstdlib>
#include <string_view>

int main(int argc, char **argv) {
    if (argc < 2) {
        return EXIT_FAILURE;
    }

    std::span<char *> args(argv, argc);
    const std::string_view commandName = args[1];

    if (commandName == "render") {
        return cblt::cli::renderCommand(args.subspan(2));
    } else if (commandName == "preprocess") {
        return cblt::cli::preprocessCommand(args.subspan(2));
    }

    return EXIT_SUCCESS;
}
