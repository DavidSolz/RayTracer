#include "MaterialBuilder.h"

MaterialBuilder::MaterialBuilder(const RenderingContext * _context){
    this->context = (RenderingContext *)_context;
    ClearMaterial();
}       

void MaterialBuilder::ClearMaterial(){
    temporaryMaterial.albedo = {0};
    temporaryMaterial.diffuse = {0};
    temporaryMaterial.specular = {1.0f, 1.0f, 1.0f, 1.0f};
    temporaryMaterial.transmissionFilter = {1.0f, 1.0f, 1.0f, 1.0f};
    temporaryMaterial.specularIntensity = 1.0f;
    temporaryMaterial.transparency = 0.0f;
    temporaryMaterial.indexOfRefraction = 1.45f; 
    temporaryMaterial.roughness = 0.5f;
    temporaryMaterial.metallic = 0.0f;
    temporaryMaterial.sheen = 0.0f;
    temporaryMaterial.anisotropy = 0.0f;
    temporaryMaterial.anisotropyRotation = 0.0f;
    temporaryMaterial.emmissionIntensity = 0.0f;
    temporaryMaterial.anisotropy = 0.0f;
    temporaryMaterial.anisotropyRotation = 0.0f;
}

MaterialBuilder * MaterialBuilder::SetBaseColor(const Color & _color){
    temporaryMaterial.albedo = _color;
    return this;
}


MaterialBuilder * MaterialBuilder::SetSpecularColor(const Color & _color){
    temporaryMaterial.specular = _color;
    return this;
}

MaterialBuilder * MaterialBuilder::SetTransmissionFilter(const Color & _color){
    temporaryMaterial.transmissionFilter = _color;
    return this;
}


MaterialBuilder * MaterialBuilder::SetSheen(const float & _factor){
    temporaryMaterial.sheen = std::fmax(0.0f, std::fmin(_factor, 1.0f));
    return this;
}


MaterialBuilder * MaterialBuilder::SetRefractiveIndex(const float & _factor){
    temporaryMaterial.indexOfRefraction = std::fmax(1e-6f, _factor);
    return this;
}

MaterialBuilder * MaterialBuilder::SetTransparency(const float & _factor){
    temporaryMaterial.transparency = std::fmax(0.0f, std::fmin(_factor, 1.0f));
    return this;
}


MaterialBuilder * MaterialBuilder::SetDiffusionColor(const Color & _color){
    temporaryMaterial.diffuse = _color;
    return this;
}

MaterialBuilder * MaterialBuilder::SetBaseColor(const uint8_t & _R, const uint8_t & _G, const uint8_t & _B){
    Color color = {_R/255.0f, _G/255.0f, _B/255.0f};
    temporaryMaterial.albedo = color;
    return this;
}


MaterialBuilder * MaterialBuilder::SetSpecularColor(const uint8_t & _R, const uint8_t & _G, const uint8_t & _B){
    Color color = {_R/255.0f, _G/255.0f, _B/255.0f};
    temporaryMaterial.specular = color;
    return this;
}

MaterialBuilder * MaterialBuilder::SetDiffusionColor(const uint8_t & _R, const uint8_t & _G, const uint8_t & _B){
    Color color = {_R/255.0f, _G/255.0f, _B/255.0f};
    temporaryMaterial.diffuse = color;
    return this;
}

MaterialBuilder * MaterialBuilder::SetTransmissionFilter(const uint8_t & _R, const uint8_t & _G, const uint8_t & _B){
    Color color = {_R/255.0f, _G/255.0f, _B/255.0f};
    temporaryMaterial.transmissionFilter = color;
    return this;
}

MaterialBuilder * MaterialBuilder::SetSmoothness(const float & _factor){
    temporaryMaterial.metallic = std::fmax(0.0f, std::fmin(_factor, 1.0f));
    return this;
}

MaterialBuilder * MaterialBuilder::SetRoughness(const float & _factor){
    temporaryMaterial.roughness = std::fmax(0.0f, std::fmin(_factor, 1.0f));
    return this;
}

MaterialBuilder * MaterialBuilder::SetEmission(const float & _factor){
    temporaryMaterial.emmissionIntensity = std::fmax(_factor, 0.0f);
    return this;
}

MaterialBuilder * MaterialBuilder::SetSpecularIntensity(const float & _factor){
    temporaryMaterial.specularIntensity = std::fmax(0.0f, std::fmin(_factor, 1.0f));
    return this;
}

uint32_t MaterialBuilder::Build(){

    context->materials.push_back(temporaryMaterial);
    ClearMaterial();
    return context->materials.size()-1;
}


uint32_t MaterialBuilder::FindSimilarMaterial(){

    //TODO

    return UINT32_MAX;
}