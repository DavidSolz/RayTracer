#ifndef SCENESERIALIZER_H
#define SCENESERIALIZER_H

#include <fstream>
#include <cstring>
#include <string>
#include <vector>

#include "RenderingContext.h"
#include "MaterialBuilder.h"

class SceneSerializer{
private:

    const char * properties[OBJECT_PROPERTIES_SIZE] = OBJECT_PROPERTIES;
    const char * types[SPATIAL_TYPE_SIZE] = SPATIAL_TYPES;

    struct MaterialInfo{
        std::string materialname;
        uint32_t materialID;
    };

    RenderingContext * context;
    MaterialBuilder * materialBuilder;

    std::fstream file;
    std::vector<MaterialInfo> materials;

    Object temporaryObject;

    bool CheckBrackets(const char * data, const size_t & size);

    const char * CheckProperties(const char * data);

    bool CheckTypes(const char * data);

    std::vector<std::string>& Tokenize(const char * data, const char * delimiter = " ");

    void ResetObject();

    void ParseObject(const std::vector<std::string> & tokens);

    void ParseScene(const char * data);

public:

    SceneSerializer(RenderingContext * _context);

    void SaveToFile( const char * _filename);

    void LoadFromFile(const char * _filename);

    ~SceneSerializer();
};

#endif