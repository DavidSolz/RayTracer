#ifndef MESH_H
#define MESH_H

#include <stdint.h>
#include <vector>

#include "Object.h"

struct Mesh{

    uint32_t numVertices;
    uint32_t numIndices;

    std::vector<Vector3> vertices;
    std::vector<int> indices;
};

#endif

