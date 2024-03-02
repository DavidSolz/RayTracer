#ifndef MESHREADER_H
#define MESHREADER_H

#define BUFFER_SIZE 100

#include "Serializer.h"
#include "RenderingContext.h"
#include "MaterialSerializer.h"

class MeshSerializer : public Serializer{
private:

    struct Face {
        int32_t indices[3];
        int32_t texels[3];
        int32_t normals[3];
        uint32_t materialID;
    };

    uint32_t currentMaterial;

    std::vector<Face> faces;
    std::vector<Vector3> vertices;
    std::vector<Vector3> normals;

    RenderingContext * context;
    MaterialSerializer * materialSerializer;

    void ParseFace(const  std::vector<std::string> & tokens);

    void BuildTriangles();

    void CalculateNormals();

    void Parse(std::ifstream & file, const char * filename);

public:

    MeshSerializer(RenderingContext * _context, MaterialSerializer * _materalSerializer);

    void LoadFromFile(const char * _filename);

};

#endif