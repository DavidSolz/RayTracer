#include "resources/KernelStructs.h"

void kernel Transfer(
    global struct Resources * resources,
    global struct Object * objects,
    global struct Material * materials,
    global const float3 * vertices,
    global struct Texture * textureInfo,
    const int numObject,
    const int numMaterials
    ){

    resources->objects = objects;
    resources->materials = materials;
    resources->vertices = vertices;
    resources->textureInfo = textureInfo;
    resources->numObject = numObject;
    resources->numMaterials = numMaterials;
}