#ifndef CBLT_CLI_RENDER_H
#define CBLT_CLI_RENDER_H

#include "core/size_types.h"

#include <optional>
#include <string>

namespace cblt::cli {

struct RenderTarget {
    uint32_t offsetX;
    uint32_t offsetY;
    uint32_t width;
    uint32_t height;
};

struct CoCLIParams {
    // input file
    std::string inputFile;
    // output file
    std::string outputFile;
    // configuration file
    std::optional<RenderTarget> outputImageTarget;
};

} // namespace cblt::cli

#endif // CBLT_CLI_RENDER_H
