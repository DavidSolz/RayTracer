#ifndef SPATIALOBJECT_H
#define SPATIALOBJECT_H

#include "Vector3.h"
#include "Material.h"

struct SpatialObject {
    Vector3 position;
    Material material;
};


#endif