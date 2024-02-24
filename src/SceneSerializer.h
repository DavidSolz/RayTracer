#ifndef SCENESERIALIZER_H
#define SCENESERIALIZER_H

#include "Serializer.h"
#include "MaterialSerializer.h"
#include "MeshSerializer.h"

#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <filesystem>

class SceneSerializer : public Serializer{
private:

    const char * properties[OBJECT_PROPERTIES_SIZE] = OBJECT_PROPERTIES;
    const char * types[SPATIAL_TYPE_SIZE] = SPATIAL_TYPES;

    RenderingContext * context;
    MaterialSerializer * materialSerializer;
    MeshSerializer * meshSerializer;

    Object temporaryObject;

    const char * CheckProperties(const char * data);

    bool CheckTypes(const char * data);

    void ResetObject();

    void ParseObject(const std::vector<std::string> & tokens);

    void Parse(std::ifstream & file, const char * filename);

public:

    SceneSerializer(RenderingContext * _context);

    void SaveToFile( const char * _filename);

    void LoadFromFile(const char * _filename);

    ~SceneSerializer();
};

#endif