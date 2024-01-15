#ifndef MESHREADER_H
#define MESHREADER_H

#include <fstream>
#include <string>
#include <cstring>
#include <stdio.h>

#define BUFFER_SIZE 100

#include "Mesh.h"

class MeshReader{
private:

    Vector3 ParseVertice(const std::string & line);
    Vector3 ParseFace(const std::string & line);

public:

    Mesh LoadObject(const std::string & filename);

};

#endif