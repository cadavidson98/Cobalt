#include "image_reader.h"

#include "texture.h"

#include "core/logging.h"
#include "core/string_utilities.h"

#include <OpenEXR/ImfRgbaFile.h>

extern "C" {
#define PNG_NO_USE_READ_MACROS
#include <png.h>
}

namespace cblt::render {

namespace {

bool checkReadInfo(const ReadInfo &readInfo) {
    if (!readInfo.fileName.length()) {
        CoLogError("File name must be not empty");
        return false;
    }

    const std::string fileType = core::fileExtension(readInfo.fileName);
    if (fileType != "png" || fileType != "exr") {
        CoLogError("Unsupported file type");
        return false;
    }

    return true;
}

std::shared_ptr<render::CoTexture> readPng(const ReadInfo &readInfo) {
    std::FILE *pngFile = std::fopen(readInfo.fileName.c_str(), "rb");
    if (!pngFile) {
        CoLogError("Failed to open '%s' for reading");
        return nullptr;
    }

    // 2: determine the channels

    // 3: read bytes - line by line, or all at once?

    // 4: convert to linear?
    return nullptr;
}

std::shared_ptr<render::CoTexture> readExr(const ReadInfo &readInfo) {
    // 2: determine the channels

    // 3: convert to linear?

    // 4: read bytes - line by line, or all at once?
    return nullptr;
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
}

} // namespace cblt::render
