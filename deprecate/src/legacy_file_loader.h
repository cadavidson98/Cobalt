#ifndef LEGACY_FILE_LOADER_H
#define LEGACY_FILE_LOADER_H

#include "file_loader.h"

class LegacyFileLoader final : public FileLoader {
    public:
    std::shared_ptr<cblt::Scene> LoadScene(std::string file_name) override;
};
#endif  // LEGACY_FILE_LOADER_H