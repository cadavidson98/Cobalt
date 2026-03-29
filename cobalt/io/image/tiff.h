#ifndef COBALT_IO_IMAGE_TIFF_H
#define COBALT_IO_IMAGE_TIFF_H

#include "image.h"

#include "math/math_types.h"

#include <filesystem>

namespace cobalt::io::image::tiff {

bool write(const std::filesystem::path filePath, FileWriterDelegate &delegate);

} // namespace cobalt::io::image::tiff

#endif // COBALT_IO_IMAGE_TIFF_H
