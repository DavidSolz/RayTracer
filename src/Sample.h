#ifndef HITINFO_H
#define HITINFO_H

#include "Material.h"
#include "Vector3.h"

struct Sample{
    float distance;
    struct Vector3 point;
    unsigned int objectID;
} __attribute((aligned((32))));

#endif