#include "commands.h"

#include <cstdlib>
#include <string>

int main(int argc, char *argv[]) {
    if (argc <= 2) {
        return EXIT_FAILURE;
    }

    std::string commandName = argv[1];

    if (commandName == "render") {
        return cblt::cli::renderCommand(argc, argv);
    }

    return EXIT_SUCCESS;
}
