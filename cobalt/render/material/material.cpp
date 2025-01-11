#include "material.h"

namespace cblt::render {

CoMaterial::CoMaterial(Ptex::PtexPtr<PtexTexture> &&texture): _texture{texture} {

}

}  // namespace cblt:material