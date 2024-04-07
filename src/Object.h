#ifndef OBJECT_H
#define OBJECT_H

#define OBJECT_PROPERTIES_SIZE 6
#define OBJECT_PROPERTIES { "position", "material", "radius", "normal", "scale", "rotation"}

#include "Material.h"
#include "SpatialType.h"
#include "Vector3.h"

struct Object{
    SpatialType type; // 1
    float radius; // 4
    Vector3 position; //16
    Vector3 normals[3]; // 48
    Vector3 vertices[3]; //48
    Vector3 uvs[3]; //48
    int materialID; //4
} __attribute((aligned(256)));


#endif
