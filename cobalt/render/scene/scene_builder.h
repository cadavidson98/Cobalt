#ifndef CBLT_RENDER_SCENE_BUILDER_H
#define CBLT_RENDER_SCENE_BUILDER_H

#include "mesh.h"
#include "simd/simd_vec3.h"
#include "vec4.h"

#include <memory>
#include <optional>
#include <string>

namespace cblt::render {

class CoScene;

class CoSceneBuilder {
    public:
        enum SceneFormat {
            kMitsuba,
            kObj,
        };

        struct CreateInfo {
                std::string fileName;
        };

        static std::shared_ptr<CoSceneBuilder> create(const CreateInfo &createInfo);
        // todo: pass the XML to these build functions instead of as constructor argument?
        virtual bool buildMeshes() = 0;
        virtual bool buildCameras() = 0;
        virtual bool buildEnvironment() = 0;

        virtual std::shared_ptr<CoScene> scene() const = 0;

    protected:
        // obj loading
        struct MeshBuffers {
                simd::vec3f *positions;
                size_t numPositions;
                vec4u *indices;
                size_t numIndices;
        };

        struct MeshBuffersSizeInfo {
                size_t numPositions;
                size_t numNormals;
                size_t numFaces;
        };

        static std::optional<MeshBuffers> _readObjFile(const std::string &fileName);
        static std::optional<MeshBuffersSizeInfo> _scanMeshBuffersSize(std::ifstream &objFileStream);
        static vec3i _parseIndices(const std::string &faceString);
};

} // namespace cblt::render

#endif // CBLT_RENDER_SCENE_BUILDER_H
