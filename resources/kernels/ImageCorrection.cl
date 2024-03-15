#include "resources/kernels/KernelStructs.h"

void kernel ImageCorrection(
    write_only image2d_t image,
    global struct Resources * resources, 
    const int numFrames,
    const float gamma
    ){

    local struct Resources localResources;
    localResources = *resources;

    int2 coord = (int2)(get_global_id(0), get_global_id(1));

    int width = localResources.width;
    int height = localResources.height;

    float scale = 1.0f / (1.0f + numFrames);
    int index = coord.y * width + coord.x;

    localResources.colors[index] = mix(localResources.colors[index], localResources.accumulator[index], scale);
    float4 color = localResources.colors[index];
    
    color = pow(color, gamma);

    write_imagef(image, coord, color);
}
