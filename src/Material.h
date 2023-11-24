#ifndef MATERIAL_H
#define MATERIAL_H

#include "Color.h"

struct Material {
    Color baseColor;
    Color diffuse;
    Color specular;
    Color emission;
    float smoothness;
    float emmissionScale;
    float diffusionScale;
    float transparencyScale;
};


#endif
