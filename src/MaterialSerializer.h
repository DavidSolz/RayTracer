#ifndef MATERIALSERIALIZER_H
#define MATERIALSERIALIZER_H

#define DEFAULT_MATERIAL_ID 0

#include "MaterialBuilder.h"
#include "RenderingContext.h"
#include "Serializer.h"
#include "BitmapReader.h"

#include <map>

class MaterialSerializer : public Serializer{
private:

    RenderingContext * context;
    MaterialBuilder * builder;

    std::map<std::string, uint32_t> gatheredMaterials;

    void Parse(std::ifstream & file, const char * filename);

public:

    MaterialSerializer(RenderingContext * _context);
    
    uint32_t GetMaterial(const std::string & _materialName);

    void SaveToFile(const char * _filename);

    void LoadFromFile(const char * _filename);

    ~MaterialSerializer();

};

#endif