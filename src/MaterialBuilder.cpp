#include "MaterialBuilder.h"

MaterialBuilder::MaterialBuilder(const RenderingContext * _context){
    this->context = (RenderingContext *)_context;
    this->temporaryMaterial = {0};
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

MaterialBuilder * MaterialBuilder::SetEmissionColor(const Color & _color){
    temporaryMaterial.emission = _color;
    return this;
}

MaterialBuilder * MaterialBuilder::SetSmoothness(const float & _factor){
    temporaryMaterial.smoothness = _factor;
    return this;
}

MaterialBuilder * MaterialBuilder::SetEmission(const float & _factor){
    temporaryMaterial.emmissionScale = _factor;
    temporaryMaterial.diffusionScale = 1.0f - _factor;
    return this;
}

MaterialBuilder * MaterialBuilder::SetDiffusion(const float & _factor){
    temporaryMaterial.diffusionScale = _factor;
    temporaryMaterial.emmissionScale = 1.0f - _factor;
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