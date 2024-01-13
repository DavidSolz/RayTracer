#include "MaterialBuilder.h"

MaterialBuilder::MaterialBuilder(const RenderingContext * _context){
    this->context = (RenderingContext *)_context;
    this->temporaryMaterial = {0};
    
    // Default refractive index for air
    this->temporaryMaterial.refractiveIndex = 1.00029f; 
}       

MaterialBuilder * MaterialBuilder::SetBaseColor(const Color & _color){
    temporaryMaterial.baseColor = _color;
    return this;
}

MaterialBuilder * MaterialBuilder::SetDiffuseColor(const Color & _color){
    temporaryMaterial.diffuse = _color;
    return this;
}

MaterialBuilder * MaterialBuilder::SetSpecularColor(const Color & _color){
    temporaryMaterial.specular = _color;
    return this;
}

MaterialBuilder * MaterialBuilder::SetRefractiveIndex(const float & _factor){
    temporaryMaterial.refractiveIndex = _factor;
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

MaterialBuilder * MaterialBuilder::SetDiffuseColor(const uint8_t & _R, const uint8_t & _G, const uint8_t & _B){
    Color color = {_R/255.0f, _G/255.0f, _B/255.0f};
    temporaryMaterial.diffuse = color;
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
    temporaryMaterial.smoothness = std::fmax(0.0f, std::fmin(_factor, 1.0f));
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

    uint32_t mostSimilar = FindSimilarMaterial();


    context->materials.emplace_back(temporaryMaterial);
    temporaryMaterial = {0};
    return context->materials.size()-1;

    //TODO

}


uint32_t MaterialBuilder::FindSimilarMaterial(){

    for(int i=0; i < context->materials.size(); i++){

        float difference = Material::Difference(context->materials[i], temporaryMaterial);

        if(difference < EPSILON){
            return i;
        }

    }

    return UINT32_MAX;
}