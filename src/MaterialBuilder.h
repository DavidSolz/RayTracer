#ifndef MATERIALBUILDER_H
#define MATERIALBUILDER_H

#include "RenderingContext.h"

class MaterialBuilder{

public:

    MaterialBuilder(const RenderingContext * _context);

    MaterialBuilder * SetBaseColor(const Color & _color);

    MaterialBuilder * SetDiffuseColor(const Color & _color);

    MaterialBuilder * SetSpecularColor(const Color & _color);

    MaterialBuilder * SetEmissionColor(const Color & _color);

    MaterialBuilder * SetSmoothness(const float & _factor);

    MaterialBuilder * SetEmission(const float & _factor);

    MaterialBuilder * SetDiffusion(const float & _factor);

    uint32_t Build();

private:

    Material * FindTheMostSimilar();

    // This constant describes maximal material similarity difference
    const float EPSILON = 0.01f;

    Material temporaryMaterial;
    RenderingContext * context;
};



#endif