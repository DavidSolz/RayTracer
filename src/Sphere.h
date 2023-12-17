#ifndef SPHERE_H
#define SPHERE_H

#include "Material.h"

struct Sphere{
    float radius;
    Vector3 position;
    uint32_t materialID;
};

#endif
