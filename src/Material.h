#ifndef MATERIAL_H
#define MATERIAL_H

#include "Color.h"

struct Material {
    struct Color baseColor;
    struct Color diffuse;
    struct Color specular;
    struct Color emission;
    float smoothness;
    float emmissionScale;
    float specularScale;
    float diffusionScale;
    float transparencyScale;
};


#endif
