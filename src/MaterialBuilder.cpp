#include "MaterialBuilder.h"

MaterialBuilder::MaterialBuilder(const RenderingContext * _context){
    this->context = (RenderingContext *)_context;
    this->temporaryMaterial = {0};
    this->temporaryMaterial.refractiveIndex = 1.00029f; 
    this->temporaryMaterial.roughness = 1e-6f;
}       

MaterialBuilder * MaterialBuilder::SetBaseColor(const Color & _color){
    temporaryMaterial.baseColor = _color;
    return this;
}


MaterialBuilder * MaterialBuilder::SetSpecularColor(const Color & _color){
    temporaryMaterial.specular = _color;
    return this;
}

MaterialBuilder * MaterialBuilder::SetRefractiveIndex(const float & _factor){
    temporaryMaterial.refractiveIndex = std::fmax(1.0f, _factor);
    return this;
}

MaterialBuilder * MaterialBuilder::SetTransparency(const float & _factor){
    temporaryMaterial.transparency = std::fmax(0.0f, std::fmin(_factor, 1.0f));
    return this;
}


MaterialBuilder * MaterialBuilder::SetEmissionColor(const Color & _color){
    temporaryMaterial.emission = _color;
    return this;
}

MaterialBuilder * MaterialBuilder::SetBaseColor(const uint8_t & _R, const uint8_t & _G, const uint8_t & _B){
    Color color = {_R/255.0f, _G/255.0f, _B/255.0f};
    temporaryMaterial.baseColor = color;
    return this;
}


MaterialBuilder * MaterialBuilder::SetSpecularColor(const uint8_t & _R, const uint8_t & _G, const uint8_t & _B){
    Color color = {_R/255.0f, _G/255.0f, _B/255.0f};
    temporaryMaterial.specular = color;
    return this;
}

MaterialBuilder * MaterialBuilder::SetEmissionColor(const uint8_t & _R, const uint8_t & _G, const uint8_t & _B){
    Color color = {_R/255.0f, _G/255.0f, _B/255.0f};
    temporaryMaterial.emission = color;
    return this;
}

MaterialBuilder * MaterialBuilder::SetSmoothness(const float & _factor){
    temporaryMaterial.metallic = std::fmax(0.0f, std::fmin(_factor, 1.0f));
    return this;
}

MaterialBuilder * MaterialBuilder::SetRoughness(const float & _factor){
    temporaryMaterial.roughness = std::fmax(1e-6f, std::fmin(_factor, 1.0f));
    return this;
}

MaterialBuilder * MaterialBuilder::SetEmission(const float & _factor){
    temporaryMaterial.emmissionScale = std::fmax(0.0f, std::fmin(_factor, 1.0f));
    return this;
}

MaterialBuilder * MaterialBuilder::SetDiffusion(const float & _factor){
    temporaryMaterial.diffusionScale = std::fmax(0.0f, std::fmin(_factor, 1.0f));
    return this;
}

uint32_t MaterialBuilder::Build(){

    context->materials.push_back(temporaryMaterial);
    temporaryMaterial = {0};
    this->temporaryMaterial.refractiveIndex = 1.00029f; 
    this->temporaryMaterial.roughness = 1e-6f;
    return context->materials.size()-1;

}


uint32_t MaterialBuilder::FindSimilarMaterial(){

    //TODO

    return UINT32_MAX;
}