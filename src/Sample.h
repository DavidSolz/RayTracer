#ifndef HITINFO_H
#define HITINFO_H

#include "Material.h"
#include "Vector3.h"

struct Sample{
    float distance;
    struct Vector3 point;
    struct Vector3 normal;
    struct Color texel;
    unsigned int objectID;
    unsigned int materialID;
};

#endif