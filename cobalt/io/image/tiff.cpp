#include "tiff.h"

#include "core/logging.h"

#include <cstdio>
#include <vector>

extern "C" {
#include <tiffio.h>
}

namespace cobalt::io::image::tiff {

namespace {

void tiffError([[maybe_unused]] const char *module, const char *format, va_list args) {
    static constexpr size_t kMaxMessageLength = 512;
    std::array<char, kMaxMessageLength> error;
    snprintf(error.data(), kMaxMessageLength, format, args);
    CoLogError("error in TIFF library: module: %s error: %s", module, error.data());
}

struct FileHandle {
    ~FileHandle() {
        TIFFClose(file);
    }

    TIFF *file;

    explicit operator bool() const {
        return file;
    }
};

} // anonymous namespace

bool write(const std::filesystem::path filePath, FileWriterDelegate &delegate) {
    TIFFSetErrorHandler(tiffError);
    // note: not 'const' because this is a C API...
    FileHandle handle{
        .file = TIFFOpen(filePath.c_str(), "w"),
    };

    if (!handle) {
        CoLogError("failed to open TIFF file");
        return false;
    }

    const Metadata metadata = delegate.metadata();

    const int headerConfigured = TIFFSetField(handle.file, TIFFTAG_IMAGEWIDTH, metadata.size.x) &
                                 TIFFSetField(handle.file, TIFFTAG_IMAGELENGTH, metadata.size.y) &
                                 TIFFSetField(handle.file, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT) &
                                 TIFFSetField(handle.file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB) &
                                 TIFFSetField(handle.file, TIFFTAG_PRIMARYCHROMATICITIES, &metadata.colorspace.red) &
                                 TIFFSetField(handle.file, TIFFTAG_WHITEPOINT, &metadata.colorspace.whitePoint) &
                                 TIFFSetField(handle.file, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG) &
                                 TIFFSetField(handle.file, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP) &
                                 TIFFSetField(handle.file, TIFFTAG_SAMPLESPERPIXEL, metadata.channelCount) &
                                 TIFFSetField(handle.file, TIFFTAG_BITSPERSAMPLE, 32);

    if (!headerConfigured) {
        return false;
    }

    // compression
    const int stripSize = TIFFDefaultStripSize(handle.file, 0);
    TIFFSetField(handle.file, TIFFTAG_ROWSPERSTRIP, stripSize);

    std::vector<float> scanlineBytes(metadata.size.x * 3);

    for (size_t idx = 0; idx < metadata.size.y; ++idx) {
        if (!delegate.row(idx, scanlineBytes)) {
            return false;
        }

        static constexpr int kWriteFailed = -1;
        const int result = TIFFWriteScanline(handle.file, scanlineBytes.data(), idx, 0);
        if (result == kWriteFailed) {
            return false;
        }
    }

    TIFFFlush(handle.file);
    return true;
}

} // namespace cobalt::io::image::tiff
