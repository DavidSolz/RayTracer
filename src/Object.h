#ifndef OBJECT_H
#define OBJECT_H

#include "Material.h"
#include "SpatialType.h"

struct Object{
    SpatialType type;
    float radius;
    Vector3 position;
    Vector3 normal;
    uint32_t materialID;
};

#endif
