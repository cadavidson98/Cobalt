#ifndef CBLT_RENDER_TEXTURE_CACHE_H
#define CBLT_RENDER_TEXTURE_CACHE_H

#include "core/size_types.h"

#include <memory>

#include <Ptexture.h>

namespace cblt::render {

class CoTexture;

class CoTextureCache {
public:
    struct CreateInfo {
        std::string baseDirectory;
        size_t cacheSizeMB;
    };

    struct FetchInfo {
        std::string texturePath;
    };

    ~CoTextureCache();

    static std::unique_ptr<CoTextureCache> create(const CreateInfo &createInfo);

    std::weak_ptr<CoTexture> fetchTexture(const FetchInfo &textureInfo);

private:
    struct CacheNode {
        std::string texturePath = {};
        size_t textureSizeMB = {};
        std::shared_ptr<CoTexture> texture = nullptr;

        CacheNode *next = nullptr;
    };

    CoTextureCache() = delete;
    CoTextureCache(const CreateInfo &createInfo);

    size_t _maxCacheSizeMB = 0;
    std::string _baseDirectory = {};

    // NOTE: generally, both of these won't be used in the same instance
    CacheNode *_LRUCache = nullptr;
    Ptex::PtexCache *_ptexCache = nullptr;

    void _evictTextures();
    void _deleteCacheNodes(CacheNode *node);
};

} // namespace cblt::render

#endif // CBLT_RENDER_TEXTURE_CACHE_H
