#include "png_reader.h"

#include "core/logging.h"

#include <memory>

extern "C" {
// clang-format off
#define PNG_NO_USE_READ_MACROS
#include <setjmp.h>
#include <pngconf.h>
#include <png.h>
// clang-format on
}

#include <limits>

namespace cobalt::io::png {

namespace {

void pngError([[maybe_unused]] png_structp pngPtr, png_const_charp errorMessage) {
    CoLogError("Error reading PNG: %s", errorMessage);
    throw std::runtime_error(errorMessage);
}

void pngWarning([[maybe_unused]] png_structp pngPtr, png_const_charp errorMessage) {
    CoLogWarning("Warning reading PNG: %s", errorMessage);
}

} // anonymous namespace

std::optional<ByteImage> read(const std::filesystem::path filePath) {
    std::FILE *pngFile = std::fopen(filePath.c_str(), "rb");
    if (!pngFile) {
        CoLogError("Failed to open '%s' for reading");
        return std::nullopt;
    }

    // 2: determine the channels
    png_structp pngReader = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!pngReader) {
        return std::nullopt;
    }

    png_infop pngInfo = png_create_info_struct(pngReader);
    if (!pngInfo) {
        png_destroy_read_struct(&pngReader, NULL, NULL);
        return std::nullopt;
    }

    png_set_error_fn(pngReader, NULL, pngError, pngWarning);

    std::unique_ptr<png_byte[]> rowData = nullptr;

    uint32_t channelCount = 0;
    vec2u textureSize = {};
    std::shared_ptr<float[]> textureData = nullptr;

    try {
        png_init_io(pngReader, pngFile);
        // converts to linear and premultiplies alpha
        // TODO: occasionally causes build errors with 'undefined function'
        // png_set_alpha_mode(pngReader, int(PNG_ALPHA_STANDARD), double(PNG_DEFAULT_sRGB));

        // 3: read bytes - line by line, or all at once?
        png_uint_32 headerWidth = 0;
        png_uint_32 headerHeight = 0;
        int bitDepth = 0;
        int colorType = 0;

        png_read_info(pngReader, pngInfo);
        png_get_IHDR(pngReader, pngInfo, &headerWidth, &headerHeight, &bitDepth, &colorType, NULL, NULL, NULL);

        switch (colorType) {
        case PNG_COLOR_TYPE_GRAY :
            channelCount = 1;
            break;
        case PNG_COLOR_TYPE_GRAY_ALPHA :
            channelCount = 2;
            break;
        case PNG_COLOR_TYPE_PALETTE :
        case PNG_COLOR_TYPE_RGB :
            channelCount = 3;
            break;
        case PNG_COLOR_TYPE_RGB_ALPHA :
            channelCount = 4;
            break;
        default :
            channelCount = 0;
        }

        if (!channelCount) {
            CoLogError("Unsupported Pixel format for png");
            png_destroy_read_struct(&pngReader, &pngInfo, NULL);
            std::fclose(pngFile);
            return std::nullopt;
        }

        if (colorType == PNG_COLOR_TYPE_PALETTE) {
            png_set_palette_to_rgb(pngReader);
        }

        if (png_get_valid(pngReader, pngInfo, PNG_INFO_tRNS)) {
            png_set_tRNS_to_alpha(pngReader);
        }

        if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8) {
            png_set_expand_gray_1_2_4_to_8(pngReader);
        }

        if (bitDepth < 16) {
            png_set_expand_16(pngReader);
        }

        png_set_scale_16(pngReader);

        const int numPasses = png_set_interlace_handling(pngReader);
        png_read_update_info(pngReader, pngInfo);

        const size_t rowSizeBytes = png_get_rowbytes(pngReader, pngInfo);
        const size_t width = png_get_image_width(pngReader, pngInfo);
        const size_t height = png_get_image_height(pngReader, pngInfo);

        rowData = std::make_unique<png_byte[]>(rowSizeBytes);
        textureData = std::make_shared<float[]>(width * height * channelCount);
        textureSize = {uint32_t(width), uint32_t(height)};

        static constexpr size_t kBytesPerChannel = sizeof(uint16_t);
        const size_t bytesPerPixel = kBytesPerChannel * channelCount;
        static constexpr float kMaxShort = float(std::numeric_limits<uint16_t>::max());

        auto shortToFloat = [](const uint16_t shortValue) -> float {
            return float(shortValue) / kMaxShort;
        };

        for (size_t y = 0; y < height; ++y) {
            // handles interlacing
            for (int passIdx = 0; passIdx < numPasses; ++passIdx) {
                png_read_row(pngReader, rowData.get(), NULL);
            }

            // convert to final representation
            for (size_t x = 0; x < width; ++x) {
                const size_t readIndex = x * bytesPerPixel;
                const size_t writeIndex = (y * width + x) * channelCount;
                for (size_t channel = 0; channel < channelCount; ++channel) {
                    const size_t channelIndex = readIndex + (channel * kBytesPerChannel);
                    const uint16_t shortByte = uint16_t(rowData[channelIndex]) << 8 |
                                               uint16_t(rowData[channelIndex + 1]);

                    textureData[writeIndex + channel] = shortToFloat(shortByte);
                }
            }
        }

        png_read_end(pngReader, NULL);
        png_destroy_read_struct(&pngReader, &pngInfo, NULL);
        std::fclose(pngFile);
    } catch (const std::exception &error) {
        png_destroy_read_struct(&pngReader, &pngInfo, NULL);
        std::fclose(pngFile);
        return std::nullopt;
    }

    return std::make_optional<ByteImage>(textureData, channelCount, textureSize);
}

} // namespace cobalt::io::png
