#include "image_reader.h"

#include "texture.h"

#include "core/logging.h"
#include "core/size_types.h"
#include "core/string_utilities.h"
#include "math/vec4.h"

#include <Imath/ImathBox.h>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfInputFile.h>

#include <ImfFrameBuffer.h>
#include <ImfPixelType.h>
#include <half.h>
#include <pngconf.h>

extern "C" {
// clang-format off
#define PNG_NO_USE_READ_MACROS
#include <setjmp.h>
#include <png.h>
// clang-format on
}

#include <algorithm>
#include <csetjmp>
#include <limits>
#include <stdexcept>

namespace cblt::render {

namespace {

static constexpr std::array<const char *, 2> kValidFileTypes = {
    "exr",
    "png",
};

bool checkReadInfo(const ReadInfo &readInfo) {
    if (!readInfo.fileName.length()) {
        CoLogError("File name must be not empty");
        return false;
    }

    const std::string fileType = core::fileExtension(readInfo.fileName);
    if (std::find(kValidFileTypes.begin(), kValidFileTypes.end(), fileType) == kValidFileTypes.end()) {
        CoLogError("Invalid file type");
        return false;
    }

    return true;
}

void pngError(png_structp pngPtr, png_const_charp errorMessage) {
    CoLogError("Error reading PNG: %s", errorMessage);
    throw std::runtime_error(errorMessage);
}

void pngWarning(png_structp pngPtr, png_const_charp errorMessage) {
    CoLogWarning("Warning reading PNG: %s", errorMessage);
}

std::shared_ptr<render::CoTexture> readPng(const ReadInfo &readInfo) {
    std::FILE *pngFile = std::fopen(readInfo.fileName.c_str(), "rb");
    if (!pngFile) {
        CoLogError("Failed to open '%s' for reading");
        return nullptr;
    }

    // 2: determine the channels
    png_structp pngReader = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!pngReader) {
        return nullptr;
    }

    png_infop pngInfo = png_create_info_struct(pngReader);
    if (!pngInfo) {
        png_destroy_read_struct(&pngReader, NULL, NULL);
        return nullptr;
    }

    png_set_error_fn(pngReader, NULL, pngError, pngWarning);

    std::unique_ptr<png_byte[]> rowData = nullptr;

    uint32_t numChannels = 0;
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
        case PNG_COLOR_TYPE_GRAY : numChannels = 1; break;
        case PNG_COLOR_TYPE_GRAY_ALPHA : numChannels = 2; break;
        case PNG_COLOR_TYPE_PALETTE :
        case PNG_COLOR_TYPE_RGB : numChannels = 3; break;
        case PNG_COLOR_TYPE_RGB_ALPHA : numChannels = 4; break;
        default : numChannels = 0;
        }

        if (!numChannels) {
            CoLogError("Unsupported Pixel format for png");
            png_destroy_read_struct(&pngReader, &pngInfo, NULL);
            std::fclose(pngFile);
            return nullptr;
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
        textureData = std::make_shared<float[]>(width * height * numChannels);
        textureSize = {uint32_t(width), uint32_t(height)};

        static constexpr size_t kBytesPerChannel = sizeof(uint16_t);
        const size_t bytesPerPixel = kBytesPerChannel * numChannels;
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
                const size_t writeIndex = (y * width + x) * numChannels;
                for (size_t channel = 0; channel < numChannels; ++channel) {
                    const size_t channelIndex = readIndex + (channel * kBytesPerChannel);
                    const uint16_t shortByte =
                        uint16_t(rowData[channelIndex]) << 8 | uint16_t(rowData[channelIndex + 1]);

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
        return nullptr;
    }

    return CoTexture::create({
        .bytes = textureData,
        .format = CoPixelFormat::Float,
        .numChannels = numChannels,
        .dimensions = textureSize,
    });
}

std::shared_ptr<render::CoTexture> readExr(const ReadInfo &readInfo) {

    std::shared_ptr<void> textureData = nullptr;
    CoPixelFormat textureFormat = {};
    const uint32_t numChannels = 3;
    vec2u textureSize = {};

    try {
        Imf::InputFile inputFile(readInfo.fileName.c_str(), 1);
        Imf::FrameBuffer frameBuffer;

        const Imf::ChannelList &channels = inputFile.header().channels();

        const Imath::Box2i window = inputFile.header().dataWindow();
        const uint32_t width = window.max.x - window.min.x + 1;
        const uint32_t height = window.max.y - window.min.y + 1;

        const Imf::Channel *red = channels.findChannel("R");
        const Imf::Channel *green = channels.findChannel("G");
        const Imf::Channel *blue = channels.findChannel("B");

        if (!numChannels) {
            CoLogError("Exr: missing valid color channels");
            return nullptr;
        }

        if (red->type != green->type || green->type != blue->type) {
            CoLogError("Mismatch in pixel width in exr image");
            return nullptr;
        }

        textureSize = {width, height};

        const size_t allocSize = numChannels * width * height;

        if (red->type == Imf::PixelType::HALF) {
            textureData = std::make_shared<Imath::half[]>(allocSize);
        } else {
            textureData = std::make_shared<float[]>(allocSize);
        }

        uint currentChannel = 0;

        auto addSlice =
            [&frameBuffer, &textureData, &currentChannel, width](const Imf::Channel *channel, const char *name) {
                if (channel) {

                    const size_t strideBytes =
                        (channel->type == Imf::PixelType::HALF) ? sizeof(Imath::half) : sizeof(float);

                    frameBuffer.insert(
                        name,
                        Imf::Slice(
                            channel->type,
                            reinterpret_cast<char *>(textureData.get()) + strideBytes * (currentChannel++),
                            strideBytes * numChannels,
                            strideBytes * numChannels * width,
                            channel->xSampling,
                            channel->ySampling,
                            0.0
                        )
                    );
                }
            };

        addSlice(red, "R");
        addSlice(green, "G");
        addSlice(blue, "B");

        inputFile.setFrameBuffer(frameBuffer);
        inputFile.readPixels(window.min.y, window.max.y);
    } catch (Iex::BaseExc &e) {
        CoLogError(e.what());
        return nullptr;
    }

    return CoTexture::create({
        .bytes = textureData,
        .format = CoPixelFormat::Half,
        .numChannels = numChannels,
        .dimensions = textureSize,
    });
}

} // namespace

std::shared_ptr<CoTexture> readImage(const ReadInfo &readInfo) {
    if (!checkReadInfo(readInfo)) {
        return nullptr;
    }

    // 1: determine the file type
    const std::string fileExtension = core::fileExtension(readInfo.fileName);

    if (fileExtension == "png") {
        return readPng(readInfo);
    } else if (fileExtension == "exr") {
        return readExr(readInfo);
    }

    return nullptr;
}

} // namespace cblt::render
