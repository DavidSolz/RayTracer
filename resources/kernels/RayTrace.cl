
#include "resources/kernels/KernelStructs.h"
#include "resources/kernels/ColorManipulation.h"
#include "resources/kernels/Intersections.h"

// Defines

#define NUM_BOUNCES 8
#define STACK_SIZE 32

float Rand(uint * seed){
    *seed = *seed * 747796405u + 2891336453u;
    uint word = ((*seed >> ((*seed >> 28u) + 4u)) ^ *seed) * 277803737u;
    return ((word>>22u) ^ word)/(float)UINT_MAX;
}

float3 RandomDirection(uint * seed){
    float latitude = ( 2.0f * Rand(seed) - 1.0f ) * M_PI_F;
    float longitude = (Rand(seed) - 0.5f) * M_PI_F;

    float cosLatitude = cos(latitude);

    return normalize((float3)(
        cosLatitude * cos(longitude),
        cosLatitude * sin(longitude),
        sin(latitude)
    ));
}

float3 Reflect(const float3 incident, const float3 normal) {

    float3 outgoing = incident - normal * 2.0f * dot(incident, normal);
    return normalize(outgoing);

}

float3 DiffuseReflect(const float3 normal, uint * seed){

    float3 randomDirection = RandomDirection(seed);
    float cosDirection = dot(normal, randomDirection);

    return normalize(randomDirection * cosDirection + normal );

}

float3 Refract(
    const float3 incident, 
    float3 normal, 
    const float n1, 
    const float n2
    ){

    float eta = n1 / n2;
    float cosI = -dot(incident, normal);
    float cosT = 1.0f - eta*eta *(1.0f - cosI*cosI);

    if( cosT < 0.0f )
        return Reflect(incident, normal);

    float3 perpendicularRay =  eta * incident;
    float3 parallelRay = eta * cosI - sqrt(fmax(0.0f, 1.0f - cosT)) * normal;

    return normalize(perpendicularRay + parallelRay);
}

float4 ComputeColorSample(
    const struct Resources resources, 
    struct Ray * ray, 
    const struct Camera camera, 
    const struct Sample sample,
    float4 * lightSample,
    uint * seed
    ){

    global const unsigned int * textureData = resources.textureData;
    global const struct Material * materials = resources.materials;
    global const struct Object * objects = resources.objects;
    global const struct Texture * infos = resources.textureInfo;

    const float4 skyColor = (float4)(0.2f, 0.2f, 0.25f, 1.0f);

    if( sample.objectID < 0 )
        return *lightSample * skyColor;

    ray->origin = sample.point;

    struct Object object = objects[ sample.objectID ];
    struct Material material =  materials[ object.materialID ];
    struct Texture info = infos[ material.textureID ];
        
    float3 normal = object.normal;

    float3 lightVector = normalize(-ray->direction);
    float3 viewVector = normalize(camera.position - sample.point);

    float3 diffusionDirection = DiffuseReflect(normal, seed);
    float3 reflectionDirection = Reflect(ray->direction, normal);
    float3 refractionDirecton = Refract(-viewVector, normal, 1.0f, material.indexOfRefraction);
    float3 halfVector = normalize(normal + reflectionDirection);

    float3 direction = mix(diffusionDirection, reflectionDirection, material.metallic);
    ray->direction = normalize( mix(direction, refractionDirecton, material.transparency) );

    float cosLight = dot(normal, lightVector);
    float cosView = dot(normal, viewVector);
    float cosHalf = dot(normal, halfVector);

    float4 emission = material.albedo * material.emmissionIntensity;
    float4 color = GetTexturePixel(textureData, &object, info, sample.point, normal);

    float4 colorSample = emission * (*lightSample); 
    (*lightSample) *= 2.0f * cosLight * material.albedo * color; 

    return colorSample;
}

// Main

void kernel RayTrace(
    global struct Resources * resources, 
    const struct Camera camera, 
    const int numFrames
    ){

    local struct Resources localResources;
    localResources = *resources;

    uint x = get_global_id(0);
    uint y = get_global_id(1);

    uint width = get_global_size(0);
    uint height = get_global_size(1);

    uint index = y * width + x;
    uint seed = (numFrames<<16) ^ (numFrames >>13) + index;

    struct Ray ray = localResources.rays[index];
    struct Sample sample = localResources.samples[index];
    float4 lightSample = localResources.light[index];

    float4 colorSample = ComputeColorSample(localResources, &ray, camera, sample, &lightSample, &seed);

    localResources.rays[index] = ray;
    localResources.light[index] = lightSample;
    localResources.accumulator[index] += colorSample;
}
