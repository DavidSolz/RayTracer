#ifndef HITINFO_H
#define HITINFO_H

#include "Material.h"
#include "Vector3.h"

struct Sample{
    struct Vector3 point;
    int objectID;
} __attribute((aligned((32))));

#endif