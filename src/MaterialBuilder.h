#ifndef MATERIALBUILDER_H
#define MATERIALBUILDER_H

#include "RenderingContext.h"
#include "BitmapReader.h"

class MaterialBuilder{
private:

    uint32_t FindSimilarMaterial();

    // This constant describes maximal material similarity difference
    const float EPSILON = 0.1f;

    Material temporaryMaterial;
    Texture info;
    RenderingContext * context;

    void ClearMaterial();

public:

    MaterialBuilder(const RenderingContext * _context);

    MaterialBuilder * SetBaseColor(const Color & _color);

    MaterialBuilder * SetTintColor(const Color & _color);

    MaterialBuilder * SetSpecularColor(const Color & _color);

    MaterialBuilder * SetTransmissionFilter(const Color & _color);

    MaterialBuilder * SetBaseColor(const uint8_t & _R, const uint8_t & _G, const uint8_t & _B);

    MaterialBuilder * SetSpecularColor(const uint8_t & _R, const uint8_t & _G, const uint8_t & _B);

    MaterialBuilder * SetTintColor(const uint8_t & _R, const uint8_t & _G, const uint8_t & _B);

    MaterialBuilder * SetTransmissionFilter(const uint8_t & _R, const uint8_t & _G, const uint8_t & _B);

    MaterialBuilder * SetBaseColor(const float & _R, const float & _G, const float & _B);

    MaterialBuilder * SetTintColor(const float & _R, const float & _G, const float & _B);

    MaterialBuilder * SetSpecularColor(const float & _R, const float & _G, const float & _B);

    MaterialBuilder * SetTransmissionFilter(const float & _R, const float & _G, const float & _B);

    MaterialBuilder * SetClearcoatThickness(const float & _factor);

    MaterialBuilder * SetClearcoatRoughness(const float & _factor);

    MaterialBuilder * SetSmoothness(const float & _factor);

    MaterialBuilder * SetSheen(const float & _factor);

    MaterialBuilder * SetTintRoughness(const float & _factor);

    MaterialBuilder * SetRoughness(const float & _factor);

    MaterialBuilder * SetAnisotropy(const float & _factor);

    MaterialBuilder * SetAnisotropyRotation(const float & _factor);

    MaterialBuilder * SetTransparency(const float & _factor);

    MaterialBuilder * SetRefractiveIndex(const float & _factor);

    MaterialBuilder * SetEmission(const float & _factor);

    MaterialBuilder * SetSpecularIntensity(const float & _factor);

    MaterialBuilder * AttachTexture( const char * _filepath);

    uint32_t EmplaceMaterial(const Material & material);

    uint32_t Build();

};



#endif