#ifndef CBLT_RENDER_MITSUBA_BUILDER_H
#define CBLT_RENDER_MITSUBA_BUILDER_H

#include "scene_builder.h"
#include "simd/simd_mat4.h"

namespace cblt::utils {
class CoXMLElement;
} // namespace cblt::utils

namespace cblt::render {

class CoMitsubaBuilder final : public CoSceneBuilder {
    public:
        CoMitsubaBuilder(const CoSceneBuilder::CreateInfo &createInfo);

        bool buildMeshes() override;
        bool buildCameras() override;

        std::shared_ptr<CoScene> scene() const override;

    private:
        std::shared_ptr<utils::CoXML> _mitsubaXML;

        simd::mat4f _buildTransform(const utils::CoXMLElement &transformElement) const;
};

} // namespace cblt::render

#endif // CBLT_RENDER_MITSUBA_BUILDER_H
