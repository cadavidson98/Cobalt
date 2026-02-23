#include "argument_parser.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>

namespace cobalt::cli {
void printUsage();

bool loadConfiguration(const std::string &file_path, RenderConfiguration &settings, RenderTarget &outputImageTarget);

std::optional<CoCLIParams> parseArguments(int numArgs, char **argv) {
    if (numArgs == 1) {
        printUsage();
        return std::nullopt;
    }

    CoCLIParams params;
    for (int arg = 1; arg < numArgs; ++arg) {
        std::string command(argv[arg]);
        if (command.compare("-i") == 0U || command.compare("--input") == 0U) {
            std::string inputFileName = argv[++arg];
            // check for extension
            size_t extensionPos = inputFileName.find_last_of('.');
            assert(extensionPos != std::string::npos);
            std::string fileType = inputFileName.substr(extensionPos);
            continue;
        } else if (command.compare("-o") == 0U || command.compare("--output") == 0U) {
            params.outputFile = argv[++arg];
            continue;
        } else if (command.compare("-c") == 0U || command.compare("--configuration") == 0U) {
            RenderConfiguration fileConfiguration;
            RenderTarget fileRenderTarget;
            std::string inputConfigFile = argv[++arg];
            if (loadConfiguration(inputConfigFile, fileConfiguration, fileRenderTarget)) {
                params.outputImageTarget.emplace(fileRenderTarget);
                params.runtimeSettings.emplace(fileConfiguration);
            }
            continue;
        } else {
            printUsage();
            return std::nullopt;
        }
    }
    return params;
}

void printUsage() {
    std::cout << "CobaltCLI -i [input file] -o [output file] -c [renderer configuration]";
}

bool loadConfiguration(const std::string &file_path, RenderConfiguration &settings, RenderTarget &outputImageTarget) {
    std::ifstream fin(file_path);
    if (!fin.good()) {
        return false;
    }

    std::string line;
    while (!fin.eof()) {
        fin >> line;
        if (!line.compare("width:")) {
            fin >> outputImageTarget.width;
        } else if (!line.compare("height:")) {
            fin >> outputImageTarget.height;
        } else if (!line.compare("path_depth:")) {
            fin >> settings.maxPathDepth;
        } else if (!line.compare("num_samples:")) {
            fin >> settings.samplesPerPixel;
        } else if (!line.compare("tile_size:")) {
            fin >> settings.tileSize;
        } else if (!line.compare("num_threads:")) {
            fin >> settings.numThreads;
        }
    }

    return true;
}

} // namespace cobalt::cli
