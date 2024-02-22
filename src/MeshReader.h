#ifndef MESHREADER_H
#define MESHREADER_H

#include <fstream>
#include <string>
#include <cstring>
#include <stdio.h>

#define BUFFER_SIZE 100

#include "RenderingContext.h"

class MeshReader{
private:

    RenderingContext * context;

    Vector3 ParseVertice(const std::string & line);
    void ParseFace(const std::string & line, int face[3]);

    void BuildTriangles();

public:

    MeshReader(RenderingContext * _context);

    Mesh * LoadObject(const std::string & filename);

};

#endif