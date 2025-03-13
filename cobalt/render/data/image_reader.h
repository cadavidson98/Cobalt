#ifndef CBLT_RENDER_IMAGE_READER_H
#define CBLT_RENDER_IMAGE_READER_H

#include <memory>
#include <string>

namespace cblt::render {

class CoTexture;

struct ReadInfo {
    std::string fileName;
};

std::shared_ptr<CoTexture> readImage(const ReadInfo &readInfo);

} // namespace cblt::render

#endif // CBLT_RENDER_IMAGE_READER_H
