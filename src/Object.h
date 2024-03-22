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
    Vector3 normals[3];
    Vector3 vertices[3];
    Vector3 uvs[3];
    int materialID;
} __attribute((aligned(256)));


#endif
