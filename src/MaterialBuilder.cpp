#include "MaterialBuilder.h"

MaterialBuilder::MaterialBuilder(const RenderingContext * _context){
    this->context = (RenderingContext *)_context;
    ClearMaterial();
}       

void MaterialBuilder::ClearMaterial(){
    temporaryMaterial.albedo = {0};
    temporaryMaterial.tint = {1.0f,1.0f,1.0f,1.0f};
    temporaryMaterial.specular = {1.0f, 1.0f, 1.0f, 1.0f};
    temporaryMaterial.transmissionFilter = {0};
    temporaryMaterial.specularIntensity = 1.0f;
    temporaryMaterial.transparency = 0.0f;
    temporaryMaterial.indexOfRefraction = 1.0f; 
    temporaryMaterial.roughness = 0.5f;
    temporaryMaterial.metallic = 0.0f;
    temporaryMaterial.sheen = 0.0f;
    temporaryMaterial.tintRoughness = 0.5f;
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

MaterialBuilder * MaterialBuilder::SetTintColor(const Color & _color){
    temporaryMaterial.tint = _color;
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

MaterialBuilder * MaterialBuilder::SetTintColor(const uint8_t & _R, const uint8_t & _G, const uint8_t & _B){
    Color color = {_R/255.0f, _G/255.0f, _B/255.0f};
    temporaryMaterial.tint = color;
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

MaterialBuilder * MaterialBuilder::SetAnisotropy(const float & _factor){
    temporaryMaterial.anisotropy = std::fmax(0.0f, std::fmin(_factor, 1.0f));
    return this;
}

MaterialBuilder * MaterialBuilder::SetTintRoughness(const float & _factor){
    temporaryMaterial.tintRoughness = std::fmax(0.0f, std::fmin(_factor, 1.0f));
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

uint32_t MaterialBuilder::EmplaceMaterial(const Material & material){
    uint32_t idx = FindSimilarMaterial();

    if ( idx == UINT32_MAX){
        context->materials.push_back(material);
        return context->materials.size()-1;
    }
    return idx;
}

uint32_t MaterialBuilder::Build(){
    uint32_t idx = FindSimilarMaterial();

    if ( idx == UINT32_MAX){
        context->materials.push_back(temporaryMaterial);
        ClearMaterial();
        return context->materials.size()-1;
    }

    return idx;
}


uint32_t MaterialBuilder::FindSimilarMaterial(){

    float highestSimilarity = 0.0f;
    uint32_t mostSimilarIndex = UINT32_MAX;

    for( uint32_t id = 0; id < context->materials.size(); ++id ){

        Material * present = &context->materials[id];

        float dAlbedo = Color::Similarity(temporaryMaterial.albedo, present->albedo);
        float dTint = Color::Similarity(temporaryMaterial.tint, present->tint);
        float dSpecular = Color::Similarity(temporaryMaterial.specular, present->specular);
        float dFilter = Color::Similarity(temporaryMaterial.transmissionFilter, present->transmissionFilter);
        float dSpecularIntensit = fabs(temporaryMaterial.specularIntensity - present->specularIntensity);
        float dTransparency = fabs(temporaryMaterial.transparency - present->transparency);
        float dIOR = fabs(temporaryMaterial.indexOfRefraction - present->indexOfRefraction);
        float dRoughness = fabs(temporaryMaterial.roughness - present->roughness);
        float dMetallic = fabs(temporaryMaterial.metallic - present->metallic);
        float dSheen = fabs(temporaryMaterial.sheen - present->sheen);
        float dTintRoughness = fabs(temporaryMaterial.tintRoughness - present->tintRoughness);
        float dClearcoatThickness = fabs(temporaryMaterial.clearcoatThickness - present->clearcoatThickness);
        float dClearcoatRoughness = fabs(temporaryMaterial.clearcoatRoughness - present->clearcoatRoughness);
        float dEmission = fabs(temporaryMaterial.emmissionIntensity - present->emmissionIntensity);
        float dAnisotropy = fabs(temporaryMaterial.anisotropy - present->anisotropy);
        float dAnisotropyRotation = fabs(temporaryMaterial.anisotropyRotation - present->anisotropyRotation);

        float similarity = 1.0f - (dAlbedo + dTint + dSpecular + 
        dSpecularIntensit + dTransparency + dFilter + 
        dIOR + dRoughness + dTintRoughness + dEmission + 
        dMetallic + dSheen + dClearcoatThickness + 
        dClearcoatRoughness + dAnisotropy + dAnisotropyRotation)/16.0f;

        if (similarity > highestSimilarity && similarity < EPSILON) {
            highestSimilarity = similarity;
            mostSimilarIndex = id;
        }

    }

    return mostSimilarIndex;
}