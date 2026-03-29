#include "commands.h"

#include "color/pixel_buffer.h"
#include "color/rgb.h"
#include "color/xyz.h"
#include "core/size_types.h"
#include "core/system.h"
#include "io/image/image.h"
#include "io/image/tiff.h"
#include "render/renderer.h"
#include "render/scene/scene.h"
#include "render/scene/scene_builder.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

namespace cobalt::cli {

namespace {

struct Arguments {
    std::string inputFile;
    std::string outputFile;
};

template<color::rgb::Colorspace colorspace>
struct image_traits {
    using colorspace_traits = color::rgb::colorspace_traits<colorspace>;

    static constexpr io::image::ColorSpace kColorspace = {
        .whitePoint = color::xyz::chromaticity(colorspace_traits::kWhitePoint),
        .red = color::xyz::chromaticity(colorspace_traits::kRed),
        .green = color::xyz::chromaticity(colorspace_traits::kGreen),
        .blue = color::xyz::chromaticity(colorspace_traits::kBlue),
    };
};

struct ColorspaceData {
    color::rgb::Colorspace colorspace;
    io::image::ColorSpace metadata;
};

template<size_t idx>
constexpr void makeColorspaceMetadata(std::span<ColorspaceData> metadatas) {
    metadatas[idx].colorspace = color::rgb::kAllColorspaces[idx];
    metadatas[idx].metadata = image_traits<color::rgb::kAllColorspaces[idx]>::kColorspace;

    makeColorspaceMetadata<idx + 1>(metadatas);
}

template<>
constexpr void makeColorspaceMetadata<color::rgb::kColorspaceCount>(
    [[maybe_unused]] std::span<ColorspaceData> metadatas
) {
    return;
}

constexpr std::array<ColorspaceData, color::rgb::kColorspaceCount> makeColorspaceMetadata() {
    std::array<ColorspaceData, color::rgb::kColorspaceCount> data;
    makeColorspaceMetadata<0>(data);
    return data;
}

void printUsage() {
    std::cout << "render -i [input file] -o [output file] -c [renderer configuration]";
}

std::optional<Arguments> parseArguments(std::span<char *> args) {
    if (args.size() == 1) {
        printUsage();
        return std::nullopt;
    }

    Arguments arguments;
    for (size_t idx = 0; idx < args.size(); ++idx) {
        const std::string_view command = args[idx];
        if (command == "-i" || command == "--input") {
            arguments.inputFile = args[++idx];
            continue;
        } else if (command == "-o" || command == "--output") {
            arguments.outputFile = args[++idx];
            continue;
        } else {
            return std::nullopt;
        }
    }

    return arguments;
}

class ImageFileWriter final : public io::image::FileWriterDelegate {
public:
    ImageFileWriter(std::shared_ptr<color::PixelBuffer> pixelBuffer): _pixelBuffer{pixelBuffer} {
    }

    ~ImageFileWriter() = default;

    io::image::Metadata metadata() const {
        const color::rgb::Colorspace colorspace = _pixelBuffer->colorspace();

        static constexpr std::array<ColorspaceData, color::rgb::kColorspaceCount> kColorspaceMetadata =
            makeColorspaceMetadata();

        const auto colorspaceData =
            std::ranges::find_if(kColorspaceMetadata, [colorspace](const ColorspaceData &ColorspaceData) -> bool {
                return ColorspaceData.colorspace == colorspace;
            });

        const io::image::ColorSpace colorspaceMetadata = colorspaceData == kColorspaceMetadata.end()
                                                             ? image_traits<color::rgb::Colorspace::kSRGB>::kColorspace
                                                             : colorspaceData->metadata;

        static constexpr io::image::ChannelFlags kChannels = io::image::ChannelFlags(io::image::Channel::kRed) |
                                                             io::image::ChannelFlags(io::image::Channel::kGreen) |
                                                             io::image::ChannelFlags(io::image::Channel::kBlue);
        static constexpr size_t kChannelCount = 3;

        return io::image::Metadata{
            .size = _pixelBuffer->size(),
            .channelCount = kChannelCount,
            .channels = kChannels,
            .colorspace = colorspaceMetadata,
        };
    }

    bool row(size_t row, std::span<float> scanlineBytes) {
        const std::span<const color::rgb::Value> scanlineValues = _pixelBuffer->scanline(row);
        if (3 * scanlineValues.size() != scanlineBytes.size()) {
            return false;
        }

        for (size_t idx = 0; idx < scanlineValues.size(); ++idx) {
            scanlineBytes[3 * idx + 0] = scanlineValues[idx].r;
            scanlineBytes[3 * idx + 1] = scanlineValues[idx].g;
            scanlineBytes[3 * idx + 2] = scanlineValues[idx].b;
        }

        return true;
    }

private:
    std::shared_ptr<color::PixelBuffer> _pixelBuffer = {};
};

} // anonymous namespace

bool renderCommand(std::span<char *> args) {

    const std::optional<Arguments> settings = parseArguments(args);
    if (!settings) {
        printUsage();
        return false;
    }

    const uint64_t loadStart = core::time();

    std::shared_ptr<render::Scene> scene = render::SceneBuilder::buildScene({
        .fileName = settings->inputFile,
        .format = render::SceneBuilder::SceneFormat::kMitsuba,
    });

    if (!scene) {
        std::cout << "Failed to load scene" << std::endl;
        return false;
    }

    const uint64_t loadEnd = core::time();

    std::cout << "Loaded Scene in " << (loadEnd - loadStart) * 1e-6 << " ms " << std::endl;

    static constexpr uint32_t kWidth = 800;
    static constexpr uint32_t kHeight = 800;
    std::shared_ptr<color::PixelBuffer> pixelBuffer = color::PixelBuffer::create({
        .colorspace = color::rgb::Colorspace::kDCIP3,
        .size = {
                 .x = kWidth,
                 .y = kHeight,
                 },
    });

    if (!pixelBuffer) {
        std::cerr << "no render target" << std::endl;
        return false;
    }

    const uint64_t renderStartTime = core::time();

    if (!render::render(scene, pixelBuffer)) {
        std::cerr << "Failed to render scene";
        return false;
    }

    const uint64_t renderEndTime = core::time();

    std::cout << "Rendered image in " << (renderEndTime - renderStartTime) * 1e-9 << " s\n";

    ImageFileWriter delegate(pixelBuffer);
    const bool wrote = io::image::tiff::write(settings->outputFile, delegate);

    if (!wrote) {
        std::cerr << "failed to write" << std::endl;
        return false;
    }

    std::cout << "wrote image " << settings->outputFile << std::endl;
    return true;
}

} // namespace cobalt::cli
