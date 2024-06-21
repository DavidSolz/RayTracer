
#include "resources/kernels/KernelStructs.h"

kernel void DepthMapping(
    global struct Resources * resources,
    global struct Ray * rays,
    global struct Sample * samples,
    global float * depth
    ){

    local struct Resources localResources;
    localResources = *resources;

    uint x = get_global_id(0);
    uint y = get_global_id(1);

    uint index = y * localResources.width + x;

    struct Sample sample = samples[index];

    if( sample.objectID < 0)
        return;

    struct Ray ray = rays[index];

    depth[index] = length(sample.point - ray.origin);

}
