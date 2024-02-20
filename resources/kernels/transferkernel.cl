#include "resources/kernels/KernelStructs.h"
void kernel Transfer(
    global struct Resources * resources,
    global const struct Object * objects,
    global const struct Material * materials,
    global const float3 * vertices,
    global const struct Texture * textureInfo,
    global const int * textureData,
    const int numObject,
    const int numMaterials
    ){

    resources->objects = objects;
    resources->materials = materials;
    resources->vertices = vertices;
    resources->textureInfo = textureInfo;
    resources->textureData = textureData;
    resources->numObject = numObject;
    resources->numMaterials = numMaterials;
}