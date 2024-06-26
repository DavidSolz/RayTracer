#include "resources/kernels/KernelStructs.h"
void kernel Transfer(
    global struct Resources * resources,
    global const struct Object * objects,
    global const struct Material * materials,
    global const struct Texture * textureInfo,
    global const unsigned int * textureData,
    global const struct BoundingBox * boxes,
    const int numObject,
    const int numMaterials,
    const int width,
    const int height
    ){

    resources->objects = objects;
    resources->materials = materials;
    resources->textureInfo = textureInfo;
    resources->textureData = textureData;
    resources->boxes = boxes;
    resources->numObject = numObject;
    resources->numMaterials = numMaterials;
    resources->width = width;
    resources->height = height;
}
