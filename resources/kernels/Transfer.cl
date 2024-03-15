#include "resources/kernels/KernelStructs.h"
void kernel Transfer(
    global struct Resources * resources,
    global const struct Object * objects,
    global const struct Material * materials,
    global const struct Texture * textureInfo,
    global const unsigned int * textureData,
    global const struct BoundingBox * boxes,
    global struct Ray * rays,
    global struct Sample * samples,
    global float4 * light,
    global float4 * accumulator,
    global float4 * colors,
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
    resources->rays = rays;
    resources->samples = samples;
    resources->light = light;
    resources->accumulator = accumulator;
    resources->colors = colors;
    resources->numObject = numObject;
    resources->numMaterials = numMaterials;
    resources->width = width;
    resources->height = height;
}