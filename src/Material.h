#ifndef MATERIAL_H
#define MATERIAL_H

#include "Color.h"

struct Material {
    Color ambient;
    Color diffuse;
    Color specular;
    Color emission;
    float shininess;
    float diffuseLevel;
};


#endif