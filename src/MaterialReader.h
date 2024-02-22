#ifndef MATERIALREADER_H
#define MATERIALREADER_H

#include "MaterialBuilder.h"
#include "RenderingContext.h"

#include <map>
#include <vector>
#include <cstring>
#include <string>
#include <fstream>

class MaterialReader{
private:
    RenderingContext * context;
    MaterialBuilder * builder;

    std::map<std::string, uint32_t> gatheredMaterials;

    std::vector<std::string>& Tokenize(const std::string & data);

    void ParseMaterial(std::ifstream & file);

public:

    MaterialReader(RenderingContext * _context);
    
    uint32_t GetMaterial(const std::string & _materialName);

    void SaveToFile(const char * _filename);

    void LoadFromFile(const char * _filename);

    ~MaterialReader();

};

#endif