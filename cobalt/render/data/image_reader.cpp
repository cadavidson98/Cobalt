#include "image_reader.h"

#include "byte_texture.h"
#include "texture.h"

#include "core/logging.h"
#include "core/size_types.h"
#include "core/string_utilities.h"
#include "math/math_types.h"

#include <Imath/ImathBox.h>
#include <Imath/half.h>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfFrameBuffer.h>
#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfPixelType.h>

#include <algorithm>
#include <csetjmp>
#include <limits>
#include <stdexcept>

namespace cblt::render {

namespace {

bool checkReadInfo(const ReadInfo &readInfo) {
    if (!readInfo.fileName.length()) {
        CoLogError("File name must be not empty");
        return false;
    }

    return true;
}

std::shared_ptr<render::CoTexture> readExr(const ReadInfo &readInfo) {

    std::shared_ptr<void> textureData = nullptr;
    CoPixelFormat textureFormat = CoPixelFormat::Half;
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
            textureFormat = CoPixelFormat::Half;
        } else {
            textureData = std::make_shared<float[]>(allocSize);
            textureFormat = CoPixelFormat::Float;
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
        return nullptr;
    }

    return CoByteTexture::create({
        .bytes = textureData,
        .format = textureFormat,
        .numChannels = numChannels,
        .dimensions = textureSize,
    });
}

} // namespace

std::shared_ptr<CoTexture> readImage(const ReadInfo &readInfo) {
    if (!checkReadInfo(readInfo)) {
        return nullptr;
    }

    const std::string fileExtension = core::fileExtension(readInfo.fileName);

    if (fileExtension == "exr") {
        return readExr(readInfo);
    }

    return nullptr;
}

} // namespace cblt::render
