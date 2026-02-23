#include "exr_reader.h"

#include "core/logging.h"

#include <Imath/ImathBox.h>
#include <Imath/half.h>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfPixelType.h>

namespace cobalt::io::exr {

std::optional<Image> read(const std::filesystem::path filePath) {
    std::shared_ptr<void> textureData = nullptr;
    const uint32_t numChannels = 3;
    vec2u textureSize = {};
    Imf::PixelType type = Imf::PixelType::HALF;

    try {
        Imf::InputFile inputFile(filePath.c_str(), 1);
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
            return std::nullopt;
        }

        if (red->type != green->type || green->type != blue->type) {
            CoLogError("Mismatch in pixel width in exr image");
            return std::nullopt;
        }

        textureSize = {width, height};

        const size_t allocSize = numChannels * width * height;

        if (red->type == Imf::PixelType::HALF) {
            textureData = std::make_shared<Imath::half[]>(allocSize);
        } else {
            textureData = std::make_shared<float[]>(allocSize);
            type = Imf::PixelType::FLOAT;
        }

        uint currentChannel = 0;

        auto addSlice =
            [&frameBuffer, &textureData, &currentChannel, width](const Imf::Channel *channel, const char *name) {
                if (channel) {

                    const size_t strideBytes = (channel->type == Imf::PixelType::HALF) ? sizeof(Imath::half)
                                                                                       : sizeof(float);

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
        return std::nullopt;
    }

    return Image{
        .data = textureData,
        .format = type,
        .size = textureSize,
        .channelCount = 3,
    };
}

} // namespace cobalt::io::exr
