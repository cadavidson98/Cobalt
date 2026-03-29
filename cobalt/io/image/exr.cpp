#include "exr.h"

#include "core/logging.h"
#include "image/image.h"

#include <Imath/ImathBox.h>
#include <Imath/half.h>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfPixelType.h>
#include <OpenEXR/ImfRgba.h>
#include <OpenEXR/ImfRgbaFile.h>

#include <algorithm>
#include <memory>

#include <ImfHeader.h>

namespace cobalt::io::image::exr {

bool read(const std::filesystem::path filePath, FileReaderDelegate &delegate) {
    try {
        Imf::RgbaInputFile inputFile(filePath.c_str(), 1);

        const Imath::Box2i window = inputFile.header().dataWindow();
        const uint32_t width = window.max.x - window.min.x + 1;
        const uint32_t height = window.max.y - window.min.y + 1;

        const vec2u textureSize = {
            .x = width,
            .y = height,
        };

        static constexpr size_t kChannelCount = 3;
        const size_t scanlineElementCount = kChannelCount * width;

        std::unique_ptr<float[]> rowFloats = std::make_unique<float[]>(scanlineElementCount);
        std::unique_ptr<Imf::Rgba[]> rowPixels = std::make_unique<Imf::Rgba[]>(width);

        const bool readMetadata = delegate.readMetadata({
            .size = textureSize,
            .channelCount = kChannelCount,
            .channels = ChannelFlags(Channel::kRed) | ChannelFlags(Channel::kGreen) | ChannelFlags(Channel::kBlue),
            .colorspace = {},
        });

        if (!readMetadata) {
            return false;
        }

        const std::span<float> floats = {
            rowFloats.get(),
            scanlineElementCount,
        };

        for (int rowIdx = window.min.y; rowIdx < window.max.y; ++rowIdx) {
            inputFile.setFrameBuffer(rowPixels.get() - rowIdx * width, 1, width);
            inputFile.readPixels(rowIdx);

            for (size_t idx = 0; idx < width; ++idx) {
                rowFloats[kChannelCount * idx + 0] = float(rowPixels[idx].r);
                rowFloats[kChannelCount * idx + 1] = float(rowPixels[idx].g);
                rowFloats[kChannelCount * idx + 2] = float(rowPixels[idx].b);
            }

            if (!delegate.readRow(rowIdx - window.min.y, floats)) {
                return false;
            }
        }
    } catch (Iex::BaseExc &e) {
        CoLogError("EXR: %s", e.what());
        return false;
    }

    return true;
}

bool write(const std::filesystem::path filePath, FileWriterDelegate &delegate) {
    try {
        const Metadata metadata = delegate.metadata();
        if (metadata.channelCount != 3) {
            CoLogError("Invalid data format for exr image: expected '3' channels, got '%zu'", metadata.channelCount);
            return false;
        }

        Imf::RgbaOutputFile outputFile(filePath.c_str(), metadata.size.x, metadata.size.y, Imf::WRITE_RGB);

        std::unique_ptr<float[]> rowFloats = std::make_unique<float[]>(metadata.size.x * metadata.channelCount);
        std::span<float> scanlineFloats = {
            rowFloats.get(),
            metadata.size.x * metadata.channelCount,
        };

        std::unique_ptr<Imf::Rgba[]> outputScanline = std::make_unique<Imf::Rgba[]>(metadata.size.x);
        outputFile.setFrameBuffer(outputScanline.get(), 1, 0);

        for (size_t rowIdx = 0; rowIdx < metadata.size.y; ++rowIdx) {
            if (!delegate.row(rowIdx, scanlineFloats)) {
                return false;
            }

            for (size_t idx = 0; idx < metadata.size.x; ++idx) {
                outputScanline[idx] = Imf::Rgba(
                    Imath::half(rowFloats[3 * idx + 0]),
                    Imath::half(rowFloats[3 * idx + 1]),
                    Imath::half(rowFloats[3 * idx + 2])
                );
            }

            outputFile.writePixels(1);
        }

    } catch (const std::exception &error) {
        CoLogError("Error writing EXR file: '%s'", error.what());
        return false;
    }

    return true;
}

} // namespace cobalt::io::image::exr
