#ifndef COBALT_CLI_PARSER_H
#define COBALT_CLI_PARSER_H

#include <cstdint>
#include <optional>
#include <string>

namespace cblt::cli {
struct RenderTarget {
        uint32_t offsetX;
        uint32_t offsetY;
        uint32_t width;
        uint32_t height;
};

struct RenderConfiguration {
        uint32_t maxPathDepth;
        uint32_t numThreads;
        uint32_t samplesPerPixel;
        uint32_t tileSize;
};

struct CoCLIParams {
        // input file
        std::string inputFile;
        // output file
        std::string outputFile;
        // configuration file
        std::optional<RenderTarget> outputImageTarget;
        std::optional<RenderConfiguration> runtimeSettings;
};

std::optional<CoCLIParams> parseArguments(int numArgs, char **argv);
} // namespace cblt::cli

#endif // COBALT_CLI_PARSER_H
