#ifndef OBJECT_H
#define OBJECT_H

#include "Material.h"
#include "SpatialType.h"
#include "Vector3.h"

struct Object{
    SpatialType type;
    float radius;
    Vector3 position;
    Vector3 normal;
    Vector3 maxPos;
    Vector3 indicesID;
    Vector3 normalsID;
    uint32_t materialID;
} __attribute((aligned(128)));


#endif
