#include "texture_cache.h"

#include "image_reader.h"
#include "texture.h"

#include "core/logging.h"
#include "core/string_utilities.h"

#include <memory>

#include <Ptexture.h>

namespace cblt::render {

namespace {

bool checkCreateInfo(const CoTextureCache::CreateInfo &createInfo) {
    if (!createInfo.cacheSizeMB) {
        CoLogWarning("CoTextureCache: cacheSizeMB == 0. Caching will be disabled");
    }

    return true;
}

} // namespace

std::unique_ptr<CoTextureCache> CoTextureCache::create(const CreateInfo &createInfo) {
    if (!checkCreateInfo(createInfo)) {
        return nullptr;
    }

    std::unique_ptr<CoTextureCache> cache(new CoTextureCache(createInfo));
    return cache;
}

CoTextureCache::CoTextureCache(const CreateInfo &createInfo)
    : _maxCacheSizeMB{createInfo.cacheSizeMB}, _baseDirectory{createInfo.baseDirectory} {
    static constexpr size_t kMBToBytes = 1024 * 1024;
    _ptexCache = Ptex::PtexCache::create(0, _maxCacheSizeMB * kMBToBytes);
}

CoTextureCache::~CoTextureCache() {
    _deleteCacheNodes(_LRUCache);

    if (_ptexCache) {
        _ptexCache->release();
    }
}

std::weak_ptr<CoTexture> CoTextureCache::fetchTexture(const FetchInfo &textureInfo) {
    const std::string filePath = (_baseDirectory.length())
                                     ? core::appendFileToPath(_baseDirectory, textureInfo.texturePath)
                                     : textureInfo.texturePath;

    CacheNode *prev = _LRUCache;
    CacheNode *iterator = _LRUCache;
    size_t cacheSizeMB = 0;
    while (iterator) {
        if (iterator->texturePath == filePath) {
            prev->next = iterator->next;
            iterator->next = _LRUCache;
            _LRUCache = iterator;
            return iterator->texture;
        }
        cacheSizeMB += iterator->textureSizeMB;
        prev = iterator;
        iterator = iterator->next;
    }

    std::shared_ptr<CoTexture> texture = readImage({
        .fileName = filePath,
    });

    if (!texture) {
        return std::weak_ptr<CoTexture>();
    }

    CacheNode *cacheNode = new CacheNode{
        .texturePath = filePath,
        .textureSizeMB = texture->size_bytes(),
        .texture = texture,
        .next = _LRUCache,
    };

    _LRUCache = cacheNode;

    if (_LRUCache->textureSizeMB + cacheSizeMB > _maxCacheSizeMB) {
        _evictTextures();
    }

    return texture;
}

void CoTextureCache::_evictTextures() {
    CacheNode *iterator = _LRUCache;
    size_t cacheSizeMB = 0;
    while (iterator) {
        cacheSizeMB += iterator->textureSizeMB;
        if (cacheSizeMB > _maxCacheSizeMB) {
            break;
        }

        iterator = iterator->next;
    }

    _deleteCacheNodes(iterator);
}

void CoTextureCache::_deleteCacheNodes(CacheNode *node) {
    CacheNode *iterator = node;
    while (iterator) {
        CacheNode *temp = iterator->next;
        delete iterator;
        iterator = temp;
    }
}

} // namespace cblt::render
