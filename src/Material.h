#ifndef MATERIAL_H
#define MATERIAL_H

#include "Color.h"
#include <stdint.h>

struct Material {
    struct Color baseColor;
    struct Color specular;
    struct Color emission;
    float metallic;
    float roughness;
    float emmissionScale;
    float diffusionScale;
    float transparency;
    float refractiveIndex;
} __attribute__((aligned(128)));

#endif
