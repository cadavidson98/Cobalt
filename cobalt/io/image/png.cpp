#include "png.h"

#include "core/logging.h"
#include "core/system.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <exception>
#include <limits>
#include <memory>
#include <ranges>
#include <string_view>

extern "C" {
// clang-format off
#define PNG_NO_USE_READ_MACROS
#ifdef PNG_SETJMP_SUPPORTED
#include <setjmp.h>
#endif  // PNG_SETJMP_SUPPORTED
#include <png.h>
// clang-format on
}

namespace cobalt::io::image::png {

namespace {

void pngError([[maybe_unused]] png_structp pngPtr, [[maybe_unused]] png_const_charp errorMessage) {
    CoLogError("Error reading PNG: %s", errorMessage);
    throw std::runtime_error(errorMessage);
}

void pngWarning([[maybe_unused]] png_structp pngPtr, [[maybe_unused]] png_const_charp errorMessage) {
    CoLogWarning("Warning reading PNG: %s", errorMessage);
}

struct FileHandle {
    ~FileHandle() {
        png_read_end(pngReader, NULL);
        png_destroy_read_struct(&pngReader, &pngInfo, NULL);
        std::fclose(pngFile);
    }

    png_structp pngReader;
    png_infop pngInfo;
    std::FILE *pngFile;
};

} // anonymous namespace

bool read(const std::filesystem::path filePath, FileReaderDelegate &delegate) {
    png_structp pngReader = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!pngReader) {
        return false;
    }

    png_infop pngInfo = png_create_info_struct(pngReader);
    if (!pngInfo) {
        png_destroy_read_struct(&pngReader, NULL, NULL);
        return false;
    }

    std::FILE *pngFile = std::fopen(filePath.c_str(), "rb");
    if (!pngFile) {
        CoLogError("Failed to open '%s' for reading");
        png_destroy_read_struct(&pngReader, &pngInfo, NULL);
        return false;
    }

    FileHandle handles = {
        .pngReader = pngReader,
        .pngInfo = pngInfo,
        .pngFile = pngFile,
    };

    png_set_error_fn(pngReader, NULL, pngError, pngWarning);

    std::unique_ptr<png_byte[]> rowData = nullptr;
    std::unique_ptr<float[]> rowFloatData = nullptr;

    try {
        png_init_io(pngReader, pngFile);

        png_uint_32 headerWidth = 0;
        png_uint_32 headerHeight = 0;
        int bitDepth = 0;
        int colorType = 0;

        png_read_info(pngReader, pngInfo);
        png_get_IHDR(pngReader, pngInfo, &headerWidth, &headerHeight, &bitDepth, &colorType, NULL, NULL, NULL);

        uint32_t channelCount = 0;
        ChannelFlags channels = 0;
        switch (colorType) {
        case PNG_COLOR_TYPE_PALETTE :
            [[fallthrough]];
        case PNG_COLOR_TYPE_RGB :
            channelCount = 3;
            channels = ChannelFlags(Channel::kRed) | ChannelFlags(Channel::kGreen) | ChannelFlags(Channel::kBlue);
            break;
        case PNG_COLOR_TYPE_RGB_ALPHA :
            channelCount = 4;
            channels = ChannelFlags(Channel::kRed) | ChannelFlags(Channel::kGreen) | ChannelFlags(Channel::kBlue) |
                       ChannelFlags(Channel::kAlpha);
            break;
        default :
            channelCount = 0;
        }

        if (!channelCount) {
            CoLogError("Unsupported Pixel format for png");
            return false;
        }

        const char *version = png_get_libpng_ver(pngReader);
        const size_t length = std::strlen(version);
        [[maybe_unused]] const std::string_view view(version, length);

        CoLogDebug("Using LibPNG version: %s", view.data());

        float gamma = 1.f;

        if (png_get_valid(pngReader, pngInfo, PNG_INFO_gAMA)) {
            double gammaDouble = 0;
            png_get_gAMA(pngReader, pngInfo, &gammaDouble);
            gamma = float(gammaDouble);
            CoLogDebug("file has gamma: %f", gammaDouble);
        } else {
            CoLogDebug("File has no gamma");
        }

        ColorSpace colorspace = {
            .whitePoint =
                {
                             .x = float(.3127f),
                             .y = float(.329f),
                             },
            .red =
                {
                             .x = float(.64f),
                             .y = float(.33f),
                             },
            .green =
                {
                             .x = float(.3f),
                             .y = float(.6f),
                             },
            .blue = {
                             .x = float(.15f),
                             .y = float(.06f),
                             },
        };

        if (png_get_valid(pngReader, pngInfo, PNG_INFO_cHRM)) {
            vec2<double> whitePoint;
            vec2<double> red;
            vec2<double> green;
            vec2<double> blue;
            png_get_cHRM(
                pngReader,
                pngInfo,
                &whitePoint.x,
                &whitePoint.y,
                &red.x,
                &red.y,
                &green.x,
                &green.y,
                &blue.x,
                &blue.y
            );

            colorspace = {
                .whitePoint =
                    {
                                 .x = float(whitePoint.x),
                                 .y = float(whitePoint.y),
                                 },
                .red =
                    {
                                 .x = float(red.x),
                                 .y = float(red.y),
                                 },
                .green =
                    {
                                 .x = float(green.x),
                                 .y = float(green.y),
                                 },
                .blue = {
                                 .x = float(blue.x),
                                 .y = float(blue.y),
                                 },
            };
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
        rowFloatData = std::make_unique<float[]>(width * channelCount);

        const vec2u textureSize = {
            .x = uint32_t(width),
            .y = uint32_t(height),
        };

        const bool processedMetadata = delegate.readMetadata({
            .size = textureSize,
            .channelCount = channelCount,
            .channels = channels,
            .colorspace = colorspace,
        });

        if (!processedMetadata) {
            return false;
        }

        static constexpr float kMaxShort = float(std::numeric_limits<uint16_t>::max());
        static constexpr size_t kBytesPerChannel = sizeof(uint16_t);
        const size_t bytesPerPixel = kBytesPerChannel * channelCount;

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
                const size_t writeIndex = x * channelCount;
                for (size_t channel = 0; channel < channelCount; ++channel) {
                    const size_t channelIndex = readIndex + (channel * kBytesPerChannel);
                    const uint16_t shortByte = uint16_t(rowData[channelIndex]) << 8 |
                                               uint16_t(rowData[channelIndex + 1]);

                    rowFloatData[writeIndex + channel] = std::pow(shortToFloat(shortByte), gamma);
                }
            }

            const std::span<const float> floats = {
                rowFloatData.get(),
                width * channelCount,
            };

            const bool processedRow = delegate.readRow(y, floats);
            if (!processedRow) {
                return false;
            }
        }
    } catch (const std::exception &error) {
        return false;
    }

    return true;
}

bool writePng(const std::filesystem::path filePath, FileWriterDelegate &delegate) {
    png_structp pngWriter = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!pngWriter) {
        return false;
    }

    png_infop pngInfo = png_create_info_struct(pngWriter);
    if (!pngInfo) {
        png_destroy_write_struct(&pngWriter, NULL);
        return false;
    }

    std::FILE *pngFile = std::fopen(filePath.c_str(), "wb");
    if (!pngFile) {
        CoLogError("Failed to open '%s' for writing", filePath.c_str());
        png_destroy_write_struct(&pngWriter, &pngInfo);
        return false;
    }

    FileHandle handles = {
        .pngReader = pngWriter,
        .pngInfo = pngInfo,
        .pngFile = pngFile,
    };

    png_set_error_fn(pngWriter, NULL, pngError, pngWarning);

    std::unique_ptr<uint8_t[]> rowBytes = nullptr;
    std::unique_ptr<float[]> rowFloats = nullptr;

    try {
        png_init_io(pngWriter, png_FILE_p(pngFile));
        static constexpr png_uint_32 kBitDepth = 8;

        const Metadata metadata = delegate.metadata();

        png_set_IHDR(
            pngWriter,
            pngInfo,
            png_uint_32(metadata.size.x),
            png_uint_32(metadata.size.y),
            kBitDepth,
            PNG_COLOR_TYPE_RGB,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT
        );

        const core::DateTime imageWriteTimestamp = core::dateAndTime();

        const png_time_struct pngTime = {
            .year = imageWriteTimestamp.year,
            .month = imageWriteTimestamp.month,
            .day = imageWriteTimestamp.day,
            .hour = imageWriteTimestamp.hour,
            .minute = imageWriteTimestamp.minute,
            .second = imageWriteTimestamp.second,
        };

        png_set_tIME(pngWriter, pngInfo, png_const_timep(&pngTime));
        // TODO: Need to set gamma?
        png_set_gAMA(pngWriter, pngInfo, PNG_DEFAULT_sRGB);

        const char *pngKeyString = "Software";
        const char *pngTextString = "Cobalt Renderer";

        const png_text_struct pngText = {
            .compression = PNG_TEXT_COMPRESSION_NONE,
            .key = const_cast<char *>(pngKeyString),
            .text = const_cast<char *>(pngTextString),
            .text_length = std::strlen(pngTextString),
            .itxt_length = 0,
            .lang = NULL,
            .lang_key = NULL,
        };

        png_set_text(pngWriter, pngInfo, png_textp(&pngText), 1);

        png_write_info(pngWriter, pngInfo);

        static constexpr float kMinValue = 0;
        static constexpr float kMaxValue = 255;

        rowBytes = std::make_unique<uint8_t[]>(3 * metadata.size.x);
        rowFloats = std::make_unique<float[]>(3 * metadata.size.x);

        std::span<uint8_t> scanlineBytes = {
            rowBytes.get(),
            metadata.size.x * 3,
        };

        std::span<float> scanlineFloats = {
            rowFloats.get(),
            metadata.size.x * 3,
        };

        for (size_t rowIdx = 0; rowIdx < metadata.size.y; ++rowIdx) {
            if (delegate.row(rowIdx, scanlineFloats)) {
                return false;
            }

            std::ranges::transform(scanlineFloats, scanlineBytes.begin(), [](float value) -> uint8_t {
                return std::clamp(value * kMaxValue, kMinValue, kMaxValue);
            });

            png_write_row(pngWriter, rowBytes.get());
        }
    } catch (const std::exception &error) {
        return false;
    }

    return true;
}

} // namespace cobalt::io::image::png
