#ifndef OBJECT_H
#define OBJECT_H

#define OBJECT_PROPERTIES_SIZE 6
#define OBJECT_PROPERTIES { "position", "material", "radius", "normal", "scale", "rotation"}

#include "Material.h"
#include "SpatialType.h"
#include "Vector3.h"

struct Object{
    SpatialType type;
    float radius;
    Vector3 position;
    Vector3 normal;
    Vector3 maxPos;
    Vector3 verticeA;
    Vector3 verticeB;
    Vector3 verticeC;
    Vector3 uv;
    uint32_t materialID;
} __attribute((aligned(128)));


#endif
