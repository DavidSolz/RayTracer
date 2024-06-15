#include "resources/kernels/KernelStructs.h"

#define BATCH_SIZE 32

void kernel ImageCorrection(
    write_only image2d_t image,
    global struct Resources * resources,
    global float4 * accumulator,
    global float4 * colors,
    const int numFrames,
    const float gamma,
    global float * depth,
    global float3 * normals
    ){

    local struct Resources localResources;

    localResources = *resources;

    int2 coord = (int2)(get_global_id(0), get_global_id(1));
    int2 lcoord = (int2)(get_local_id(0), get_local_id(1));

    int width = get_global_size(0);
    int globalIndex = coord.y * width + coord.x;

    float scale = 1.0f / (1.0f + numFrames);

    float4 color = colors[globalIndex];
    color = mix(color, accumulator[globalIndex], scale);
    colors[globalIndex] = color;

    write_imagef(image, coord, color);
}
