#ifndef SCENESERIALIZER_H
#define SCENESERIALIZER_H

#include "Serializer.h"
#include "MaterialSerializer.h"
#include "MeshSerializer.h"
#include "ObjectBuilder.h"

#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <filesystem>

class SceneSerializer : public Serializer{
private:

    Vector3 scales;

    const char * properties[OBJECT_PROPERTIES_SIZE] = OBJECT_PROPERTIES;
    const char * types[SPATIAL_TYPE_SIZE] = SPATIAL_TYPES;

    RenderingContext * context;
    MaterialSerializer * materialSerializer;
    MeshSerializer * meshSerializer;

    Object temporaryObject;

    const char * CheckProperties(const char * data);

    SpatialType CheckTypes(const char * data);

    void ResetObject();

    void ParseObject(const std::vector<std::string> & tokens);

    void Parse(std::ifstream & file, const char * filename);

public:

    SceneSerializer(RenderingContext * _context);

    void LoadFromFile(const char * _filename);

    ~SceneSerializer();
};

#endif