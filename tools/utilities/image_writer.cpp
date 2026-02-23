#include "image_writer.h"

#include "color/pixel_buffer.h"
#include "core/logging.h"
#include "core/system.h"

#include <OpenEXR/ImfRgbaFile.h>

extern "C" {
// clang-format off
#define PNG_NO_USE_READ_MACROS
#include <setjmp.h>
#include <png.h>
// clang-format on
}

#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

namespace cobalt::cli {

namespace {

bool checkWriteInfo(const WriteInfo &writeInfo) {
    if (!writeInfo.fileName.length()) {
        CoLogError("Missing File name");
        return false;
    }

    if (writeInfo.type != ImageType::kPNG && writeInfo.type != ImageType::kEXR) {
        CoLogError("Unsupported file type");
        return false;
    }

    return true;
}

float knee(float base, float clamp) {
    return std::log(base * clamp + 1.f) / clamp;
}

float gamma(float linear) {
    //
    // Conversion from half to unsigned char pixel data,
    // with gamma correction.  The conversion is the same
    // as in the exrdisplay program's ImageView class,
    // except with defog, kneeLow, and kneeHigh fixed
    // at 0.0, 0.0, and 5.0 respectively.
    //
    static constexpr float kExposure = 1.f;

    static const float linearToGrayPoint = std::pow(2.f, std::clamp(kExposure + 2.47393f, -20.f, 20.f));

    float grayPointValue = std::max(0.f, linear * linearToGrayPoint);

    grayPointValue = (grayPointValue > 1.f) ? 1.f + knee(grayPointValue - 1, 0.184874f) : grayPointValue;

    return grayPointValue * 17116.5f;
}

bool writePng(const WriteInfo &writeInfo) {
    std::FILE *pngFile = std::fopen(writeInfo.fileName.c_str(), "wb");
    if (!pngFile) {
        CoLogError("Failed to open '%s' for writing", writeInfo.fileName.c_str());
        return false;
    }

    png_structp pngWriter = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!pngWriter) {
        return false;
    }

    png_infop pngInfo = png_create_info_struct(pngWriter);
    if (!pngInfo) {
        png_destroy_write_struct(&pngWriter, NULL);
        return false;
    }

    // c equivalent of 'catch' block
    if (setjmp(png_jmpbuf(pngWriter))) {
        png_destroy_write_struct(&pngWriter, &pngInfo);
        fclose(pngFile);
        return false;
    }

    png_init_io(pngWriter, png_FILE_p(pngFile));

    static constexpr png_uint_32 kBitDepth = 8;
    const color::PixelBuffer &pixelBuffer = *writeInfo.pixelBuffer.get();
    const vec2u size = pixelBuffer.size();
    png_set_IHDR(
        pngWriter,
        pngInfo,
        png_uint_32(size.x),
        png_uint_32(size.y),
        kBitDepth,
        PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
    );

    const core::DateTime imageWriteTimestamp = core::dateAndTime();

    const png_time_struct pngTime{
        .year = imageWriteTimestamp.year,
        .month = imageWriteTimestamp.month,
        .day = imageWriteTimestamp.day,
        .hour = imageWriteTimestamp.hour,
        .minute = imageWriteTimestamp.minute,
        .second = imageWriteTimestamp.second,
    };

    png_set_tIME(pngWriter, pngInfo, png_const_timep(&pngTime));

    static const char *pngKeyString = "Software";
    static const char *pngTextString = "Cobalt Renderer";

    const png_text_struct pngText{
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

    struct pngSample {
        png_byte red;
        png_byte green;
        png_byte blue;
    };

    std::vector<pngSample> rowData(size.x);
    for (size_t rowIdx = 0; rowIdx < size.y; ++rowIdx) {
        std::span<const color::rgb::Value> scanline = pixelBuffer.scanline(rowIdx);
        std::transform(scanline.begin(), scanline.end(), rowData.begin(), [](const color::rgb::Value &color) {
            return pngSample{
                png_byte(std::clamp(gamma(color.r), kMinValue, kMaxValue)),
                png_byte(std::clamp(gamma(color.g), kMinValue, kMaxValue)),
                png_byte(std::clamp(gamma(color.b), kMinValue, kMaxValue)),
            };
        });

        png_write_row(pngWriter, reinterpret_cast<const png_byte *>(rowData.data()));
    }

    png_write_end(pngWriter, pngInfo);

    png_destroy_write_struct(&pngWriter, &pngInfo);
    fclose(pngFile);
    return true;
}

bool writeExr(const WriteInfo &writeInfo) {
    try {
        const color::PixelBuffer &pixelBuffer = *writeInfo.pixelBuffer.get();
        const vec2u size = pixelBuffer.size();
        Imf::RgbaOutputFile outputFile(writeInfo.fileName.c_str(), size.x, size.y, Imf::WRITE_RGB);

        std::vector<Imf::Rgba> outputScanline(size.x);
        outputFile.setFrameBuffer(outputScanline.data(), 1, 0);

        for (size_t rowIdx = 0; rowIdx < size.y; ++rowIdx) {
            std::span<const color::rgb::Value> scanline = pixelBuffer.scanline(rowIdx);

            std::transform(
                scanline.begin(),
                scanline.end(),
                outputScanline.begin(),
                [](const color::rgb::Value &color) {
                    return Imf::Rgba(Imath::half(color.r), Imath::half(color.g), Imath::half(color.b));
                }
            );

            outputFile.writePixels(1);
        }

    } catch (const std::exception &error) {
        CoLogError("Error writing EXR file: '%s'", error.what());
        return false;
    }

    return true;
}

} // anonymous namespace

bool writeImage(const WriteInfo &writeInfo) {
    if (!checkWriteInfo(writeInfo)) {
        return false;
    }

    switch (writeInfo.type) {
    case ImageType::kPNG :
        return writePng(writeInfo);
    case ImageType::kEXR :
        return writeExr(writeInfo);
    default :
        break;
    }

    return false;
}

} // namespace cobalt::cli
