#ifndef CBLT_RENDER_H
#define CBLT_RENDER_H

#include <memory>

#include "Ptexture.h"

namespace cblt::render {

class CoMaterial {
    public:

    struct CreateInfo {
        std::string fileName;
        std::string fileExtension;
    };

    static std::shared_ptr<CoMaterial> create(const CreateInfo &createInfo);

    private:
    Ptex::PtexPtr<Ptex::PtexTexture> _texture;


    CoMaterial(Ptex::PtexPtr<PtexTexture> &&texture);
};

} // namespace cblt::render
#endif // CBLT_RENDER_H
