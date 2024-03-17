
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
    float cosT =  1.0f - eta * eta * (1.0f - cosI*cosI);

    if( cosT < 0.0f )
        return Reflect(incident, normal);

    float3 perpendicularRay =  eta * incident;
    float3 parallelRay = eta * cosI - sqrt(1.0f - cosT ) * normal;

    return normalize(perpendicularRay + parallelRay);
}

float SchlickFresnel(const float value){
    float temp = 1.0f - value;
    return temp * temp * temp * temp * temp;
}

float GTR(const float cosHalf, const float factor){

    if(factor >= 1.0f)
        return ONE_OVER_PI;

    float factorSqr = factor * factor;
    float temp = 1.0f + ( factorSqr - 1.0f)*cosHalf*cosHalf;
    return ONE_OVER_PI * (factorSqr - 1.0f)/(log(factorSqr)*temp);
}


float4 MetallicBRDF(const float cosView, const float cosLight, const float cosHalf, const float cosLightHalf, const struct Material material){

    float F0 = (material.indexOfRefraction - 1.0f)/ (material.indexOfRefraction + 1.0f);
    F0*=F0;

    float4 diffuse = material.albedo * ONE_OVER_PI;

    float alpha = material.specularIntensity;

    float4 specular = ONE_OVER_2_PI * alpha + 2.0f/ (4.0f * pow(fmax(cosHalf, 1e-6f),alpha));

    float fresnel = F0 + (1.0f - F0) * SchlickFresnel(cosLightHalf);

    return diffuse + specular*fresnel;
}


float4 DielectricBRDF(const float cosView, const float cosLight, const float cosHalf, const float cosReflection, const struct Material material){

    float4 lambert = material.albedo * ONE_OVER_PI;
    
    float FL = pow(1.0f - cosLight, 5);
    float FV = pow(1.0f - cosView, 5);

    float cosD = cosHalf * cosHalf;

    float ior = material.indexOfRefraction;

    float Rs = (cosLight - ior * cosReflection)/(cosLight + ior * cosReflection);
    float Rp = (cosReflection - ior * cosLight)/(cosReflection + ior * cosLight);

    float R = (Rs*Rs + Rp*Rp) * 0.5f;

    float4 retro = lambert * R * (FL + FV + FL*FV*(R - 1.0f));

    return lambert * (1.0f - 0.5f * FL) * (1.0f - 0.5f * FV) + retro;
}

float4 SpecularBRDF(const float cosView, const float cosLight, const float cosHalf, const float cosLightHalf, const float cosTangentHalf, const float cosBitangentHalf, const struct Material material){

    float aspect = sqrt(1.0f - 0.9f * material.anisotropy);

    float roughnessSqr = material.roughness * material.roughness;

    float alphaX = roughnessSqr / aspect;
    float alphaY = roughnessSqr * aspect;

    float factorA = cosTangentHalf/alphaX;
    float factorB = cosBitangentHalf/alphaY;

    float denominator = factorA * factorA + factorB * factorB + cosHalf * cosHalf;

    float4 anisotropy = ONE_OVER_PI * 1.0f/(alphaX * alphaY * denominator * denominator);

    return anisotropy;
}

float4 Sheen(const float cosLightHalf, const struct Material material){
    
    float fresnel = SchlickFresnel(cosLightHalf);
    
    return fresnel * material.tint * material.tintWeight;
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

    const float4 skyColor = (float4)(0.005f, 0.005f, 0.01f, 1.0f);

    if( sample.objectID < 0 )
        return *lightSample * skyColor;

    ray->origin = sample.point;

    struct Object object = objects[ sample.objectID ];
    struct Material material =  materials[ object.materialID ];
    struct Texture info = infos[ material.textureID ];
        
    float3 normal = object.normal;
    float3 tangent = normalize(cross(normal, camera.up));
    float3 bitangent = normalize(cross(normal, tangent));

    float3 lightVector = normalize(-ray->direction);
    float3 viewVector = normalize(sample.point - camera.position);

    float3 diffusionDirection = DiffuseReflect(normal, seed);
    float3 reflectionDirection = Reflect(ray->direction, normal);
    float3 refractionDirecton = Refract(viewVector, normal, 1.0f, material.indexOfRefraction);
    float3 halfVector = normalize(normal + reflectionDirection);

    float3 direction = mix(diffusionDirection, reflectionDirection, material.metallic);
    ray->direction = normalize( mix(direction, refractionDirecton, material.transparency) );

    float cosLight = fmax(0.0f, dot(normal, lightVector));
    float cosView = fmax(0.0f, -dot(normal, viewVector));
    float cosHalf = dot(normal, halfVector);
    float cosReflection = dot(normal, ray->direction);
    float cosLightHalf = dot(lightVector, halfVector);
    float cosTangentHalf = dot(tangent, halfVector);
    float cosBitangentHalf = dot(bitangent, halfVector);

    float4 emission = material.albedo * material.emmissionIntensity * fmax(cosLight, 0.0f);
    float4 dielectric = DielectricBRDF(cosView, cosLight, cosHalf, cosReflection, material);
    float4 metallic = MetallicBRDF(cosView, cosLight, cosHalf, cosLightHalf, material);
    float4 specular = SpecularBRDF(cosView, cosLight, cosHalf, cosLightHalf, cosTangentHalf, cosBitangentHalf, material);
    float4 sheen = Sheen(cosLightHalf, material);

    float4 color = GetTexturePixel(textureData, &object, info, sample.point, normal);

    float4 ambient = mix(dielectric, specular, material.transparency);
    ambient = mix(ambient, metallic, material.metallic);

    float4 colorSample = (emission + ambient + sheen /* + clearcoat */) * (*lightSample) * color; 
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