#include "resources/kernels/KernelStructs.h"

float Rand(uint * seed){
    *seed = *seed * 747796405u + 2891336453u;
    uint word = ((*seed >> ((*seed >> 28u) + 4u)) ^ *seed) * 277803737u;
    return ((word>>22u) ^ word)/(float)UINT_MAX;
}

float3 RandomDirection(uint * seed){
    float latitude = ( 2.0f * Rand(seed) - 1.0f ) * M_PI_F;
    float longitude = Rand(seed) * M_PI_F;

    float cosLatitude = cos(latitude);

    return normalize((float3)(
        cosLatitude * cos(longitude),
        cosLatitude * sin(longitude),
        sin(latitude)
    ));
}

kernel void CastRays(
    global struct Resources * resources,
    global struct Ray * rays,
    global float4 * light,
    global float4 * accumulator,
    global float * depth,
    const struct Camera camera,
    const int numFrames
    ){

    local struct Resources localResources;

    localResources = *resources;

    uint x = get_global_id(0);
    uint y = get_global_id(1);

    uint width = localResources.width;
    uint height = localResources.height;

    uint index = y * width + x;
    uint seed = (numFrames<<16) ^ (numFrames >>13) + index;

    float3 offset = RandomDirection(&seed);

    float tanHalfFOV = tan(radians(camera.fov) * 0.5f);
    float pixelXPos = (2.0f * (x + offset.x) / width - 1.0f) * camera.aspectRatio;
    float pixelYPos = (2.0f * (y + offset.y) / height - 1.0f);

    float3 pixelPosition = camera.position + (camera.front + ( camera.right * pixelXPos + camera.up * pixelYPos) * tanHalfFOV ) * camera.near;

    struct Ray ray;
    ray.origin = camera.position;
    ray.direction = normalize(pixelPosition - ray.origin);

    rays[index] = ray;
    light[index] = (float4)(1.0f, 1.0f, 1.0f, 0.0f);
    accumulator[index] = (float4)(0.0f, 0.0f, 0.0f, 0.0f);
    depth[index] = INFINITY;
}
