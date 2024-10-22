#ifndef FILE_LOADER_H
#define FILE_LOADER_H
#include <string>
#include <memory>

#include "geom/scene.h"
// Interface for File Loading- Allows for easy support of new file formats
// without needing to manipulate existing code
class FileLoader {
    public:
    virtual std::shared_ptr<cblt::Scene> LoadScene(std::string file_name) = 0;
};

#endif  // FILE_LOADER_H