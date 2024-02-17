#ifndef MATERIAL_H
#define MATERIAL_H

#include "Color.h"
#include <stdint.h>

struct Material {
    struct Color albedo;
    struct Color tint;
    struct Color specular;
    struct Color transmissionFilter;
    float specularIntensity;
    float transparency;
    float indexOfRefraction;
    float roughness;
    float metallic;
    float sheen;
    float tintRoughness;
    float clearcoatThickness;
    float clearcoatRoughness;
    float emmissionIntensity;
    float anisotropy;
    float anisotropyRotation;
    
} __attribute__((aligned(128)));

#endif
