#ifndef MATERIALBUILDER_H
#define MATERIALBUILDER_H

#include "RenderingContext.h"

class MaterialBuilder{

public:

    MaterialBuilder(const RenderingContext * _context);

    MaterialBuilder * SetBaseColor(const Color & _color);

    MaterialBuilder * SetSpecularColor(const Color & _color);

    MaterialBuilder * SetEmissionColor(const Color & _color);

    MaterialBuilder * SetBaseColor(const uint8_t & _R, const uint8_t & _G, const uint8_t & _B);

    MaterialBuilder * SetSpecularColor(const uint8_t & _R, const uint8_t & _G, const uint8_t & _B);

    MaterialBuilder * SetEmissionColor(const uint8_t & _R, const uint8_t & _G, const uint8_t & _B);

    MaterialBuilder * SetSmoothness(const float & _factor);

    MaterialBuilder * SetRoughness(const float & _factor);

    MaterialBuilder * SetTransparency(const float & _factor);

    MaterialBuilder * SetRefractiveIndex(const float & _factor);

    MaterialBuilder * SetEmission(const float & _factor);

    MaterialBuilder * SetDiffusion(const float & _factor);

    uint32_t Build();

private:

    uint32_t FindSimilarMaterial();

    // This constant describes maximal material similarity difference
    const float EPSILON = 0.001f;

    Material temporaryMaterial;
    RenderingContext * context;
};



#endif