#ifndef COBALT_IO_IMAGE_EXR_H
#define COBALT_IO_IMAGE_EXR_H

#include "image.h"

#include <filesystem>

namespace cobalt::io::image::exr {

bool read(const std::filesystem::path filePath, FileReaderDelegate &delegate);

bool write(const std::filesystem::path filePath, FileWriterDelegate &delegate);

} // namespace cobalt::io::image::exr

#endif // COBALT_IO_IMAGE_EXR_H
