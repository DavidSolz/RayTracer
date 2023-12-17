#ifndef MATERIAL_H
#define MATERIAL_H

#include "Color.h"
#include <stdint.h>

struct Material {
    struct Color baseColor;
    struct Color diffuse;
    struct Color specular;
    struct Color emission;
    float smoothness;
    float emmissionScale;
    float diffusionScale;

static float Difference(const Material & _materialA, const Material & _materialB){

    float baseSimilarity = Color::Similarity(_materialA.baseColor, _materialB.baseColor);
    float diffuseSimilarity = Color::Similarity(_materialA.baseColor, _materialB.baseColor);
    float specularSimilarity = Color::Similarity(_materialA.baseColor, _materialB.baseColor);
    float emissionSimilarity = Color::Similarity(_materialA.baseColor, _materialB.baseColor);

    float dSmoothnes = _materialA.smoothness - _materialB.smoothness;
    float dEmission = _materialA.emmissionScale - _materialB.emmissionScale;
    float dDiffusion = _materialA.diffusionScale - _materialB.diffusionScale;

    float factorSimilarity = sqrt(dSmoothnes*dSmoothnes + dEmission*dEmission + dDiffusion * dDiffusion);

    return baseSimilarity + diffuseSimilarity + specularSimilarity + emissionSimilarity + factorSimilarity;
}

};


#endif
