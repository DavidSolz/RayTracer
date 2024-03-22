
#include "resources/kernels/KernelStructs.h"
#include "resources/kernels/ColorManipulation.h"


#define ALPHA_MIN 0.001f


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

    return normalize(randomDirection  * cosDirection + normal );

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


float4 MetallicBRDF(const float cosView, const float cosLight, const float cosHalf, const float cosLightHalf, const float cosViewHalf, const struct Material material){

    float4 diffuse = mix(0.04f, material.albedo, material.metallic);

    float alpha = material.roughness * material.roughness;
    float h2 = cosHalf * cosHalf;

    float F = 0.04f + 0.96f * SchlickFresnel(cosLightHalf);
    float G = min(2.0 * cosLight * cosView / cosViewHalf, 1.0);
    float D = ONE_OVER_PI * exp( -pow( tan( acos(cosHalf) ), 2.0f) / alpha) / ( alpha * h2 * h2 );

    return (F * G * D )/ (4.0f * cosLight * cosView) + diffuse;
}


float4 DielectricBRDF(const float cosView, const float cosLight, const float cosHalf, const float cosReflection, const struct Material material){

    float4 lambert = material.albedo * ONE_OVER_PI;

    float FL = SchlickFresnel(cosLight);
    float FV = SchlickFresnel(cosView);

    float cosD = cosHalf * cosHalf;
    float R = 2.0f * material.roughness * cosD;

    float4 retro = lambert * R * (FL + FV + FL * FV * (R - 1.0f) );

    return lambert * (1.0f - 0.5f * FL) * (1.0f - 0.5f * FV) + retro;
}

float GTRAniso(const float3 halfVector, const float ax, const float ay){

    float dotHX2 = halfVector.x * halfVector.x;
    float dotHY2 = halfVector.z * halfVector.z;
    float cos2Theta = cos(halfVector.y) * cos(halfVector.y);
    float ax2 = ax * ax;
    float ay2 = ay * ay;

    return ONE_OVER_PI / (ax * ay * (dotHX2 / ax2 + dotHY2 / ay2 + cos2Theta));
}

float DiffuseFresnel(const float NdotL, const float NdotV, const float roughness) {
    float temp = NdotL * NdotV;
    float Fd90 = 0.5f + 2.0f * temp * temp * roughness;
    float result = mix(1.0f, Fd90, SchlickFresnel(NdotL));
    return result * result;
}

float SeparableSmithGGXG1BSDF(const float3 vector, const float3 halfVector, const float ax, const float ay){
    
    float cosVectorHalf = fmax(0.0f, dot(vector, halfVector));

    float theta = fabs(tan(acos(halfVector.y)));

    float a = sqrt(cos(vector.x) * cos(vector.x) * ax * ax + sin(vector.x) * sin(vector.x) * ay * ay);
    float a2Tan2Theta = a * a * theta * theta;

    float lambda = 0.5f * (-1.0f + sqrt(1.0f + a2Tan2Theta));
    return 1.0f / (1.0f + lambda);

}

void Anisotropy(const struct Material material, float * ax, float *ay){
    float aspect = sqrt(1.0f - 0.9f * material.anisotropy);

    float roughnessSqr = material.roughness * material.roughness;

    *ax = fmax(ALPHA_MIN, roughnessSqr / aspect);
    *ay = fmax(ALPHA_MIN, roughnessSqr * aspect);

}

float4 SpecularBSDF(const float cosView, const float cosLight, const float cosLightHalf, const float3 incomingVector, const float3 reflectionVector, const float3 viewVector, const float3 halfVector, const struct Material material){

    float ax, ay;
    Anisotropy(material, &ax, &ay);

    float cosViewHalf = dot(halfVector, viewVector);

    float D = GTRAniso(halfVector, ax, ay);
    float G = SeparableSmithGGXG1BSDF(incomingVector, halfVector, ax, ay);
    G *= SeparableSmithGGXG1BSDF(reflectionVector, halfVector, ax, ay);
    float F = DiffuseFresnel(cosLightHalf, cosViewHalf, material.roughness);

    float C = cosLightHalf * cosViewHalf / cosLight * cosView;

    float denominator = cosLightHalf + material.indexOfRefraction * material.indexOfRefraction * cosViewHalf;
    float T = (material.indexOfRefraction * material.indexOfRefraction) / denominator * denominator;


    return clamp(material.specular * T * C * D * G * (1.0f - F), 0.0f, 1.0f);
}

float4 Tint(const float4 albedo){
    float luminance = albedo.x * 0.3f + albedo.y * 0.6f + albedo.z;
    return mix(1.0f, albedo/luminance , luminance>0.0f);
}

float4 Sheen(const float cosLightHalf, const struct Material material){
    float4 tint = Tint(material.albedo);
    return material.albedo * mix(1.0f, tint, material.tintWeight) * SchlickFresnel(cosLightHalf) * (material.tintWeight > 0.0f);
}

float GTR(const float cosLightHalf, const float alpha){

    if( alpha >= 1.0f)
        return ONE_OVER_PI;

    float alphaSqr = alpha * alpha;
    float decAlphaSqr = alphaSqr - 1.0f;

    return ONE_OVER_PI * decAlphaSqr/(log2(alphaSqr)*(1.0f + decAlphaSqr * cosLightHalf * cosLightHalf));
}

float SeparableSmithGGXG1(const float cosine, float alpha){
    float a2 = alpha * alpha;
    return 2.0f / (1.0f + sqrt(a2 + (1 - a2) * cosine * cosine));
}


float4 ClearcoatBRDF(const float cosView, const float cosLight, const float cosHalf, const struct Material material) {

    float D = GTR(cosHalf, material.clearcoatRoughness);
    float Gl = SeparableSmithGGXG1(cosLight, 0.25f);
    float Gv = SeparableSmithGGXG1(cosView, 0.25f);
    float F = 0.04f + 0.96f * SchlickFresnel(cosHalf);

    return 0.25f * material.clearcoatThickness * D * Gl * Gv * F;
}


float4 ComputeColorSample(
    const struct Resources resources,
    struct Ray * ray,
    const struct Camera camera,
    const struct Sample sample,
    float4 * lightSample,
    float3 normal,
    uint * seed
    ){

    global const unsigned int * textureData = resources.textureData;
    global const struct Material * materials = resources.materials;
    global const struct Object * objects = resources.objects;
    global const struct Texture * infos = resources.textureInfo;

    ray->origin = sample.point;

    struct Object object = objects[ sample.objectID ];
    struct Material material =  materials[ object.materialID ];
    struct Texture info = infos[ material.textureID ];

    float3 lightVector = normalize(-ray->direction);
    float3 viewVector = normalize(sample.point - camera.position);

    float3 diffusionDirection = DiffuseReflect(normal, seed);
    float3 reflectionDirection = Reflect(viewVector, normal);
    float3 refractionDirecton = Refract(viewVector, normal, 1.45f, material.indexOfRefraction);
    
    float3 direction = mix(diffusionDirection, reflectionDirection, material.metallic);
    ray->direction = normalize( mix(direction, refractionDirecton, material.transparency) );

    float3 halfVector = normalize(normal + ray->direction);

    float cosLight = fmax(1e-6f, dot(normal, lightVector));
    float cosView = fmax(1e-6f, dot(normal, viewVector));
    float cosHalf = dot(normal, halfVector);
    float cosReflection = dot(normal, ray->direction);
    float cosLightHalf = dot(lightVector, halfVector);
    float cosViewHalf = dot(viewVector, halfVector);

    float4 emission = material.albedo * material.emmissionIntensity;
    float isEmissive = dot(emission.xyz, (float3)(1.0f, 1.0f, 1.0f)) > 0.0f ;
    
    float4 colorSample = 0.0f;
    float diffuse = (1.0f - material.metallic) * (1.0f - material.transparency);
    float transparency = (1.0f - material.metallic) * material.transparency;

    float4 dielectric = DielectricBRDF(cosView, cosLight, cosHalf, cosReflection, material);
    float4 metallic = MetallicBRDF(cosView, cosLight, cosHalf, cosLightHalf, cosViewHalf, material);
    float4 specular = SpecularBSDF(cosView, cosLight, cosLightHalf, lightVector, ray->direction, viewVector, halfVector, material);
    
    float4 clearcoat = ClearcoatBRDF(cosView, cosLight, cosHalf, material);
    float isClearcoatPresent = cosLight > 0.0f;

    float4 sheen = Sheen(cosLightHalf, material);
    float4 color = GetTexturePixel(textureData, &object, info, sample.point, normal);

    colorSample += emission * isEmissive;
    colorSample += clearcoat * isClearcoatPresent;
    colorSample += diffuse * ( dielectric * material.diffuse * color + sheen);
    colorSample += transparency * specular;
    colorSample *= *lightSample;

    (*lightSample) *= 2.0f * cosLight * material.albedo * color;

    return colorSample;
}

// Main

void kernel RayTrace(
    global struct Resources * resources,
    global struct Ray * rays,
    global struct Sample * samples,
    global float4 * light,
    global float4 * accumulator,
    global float4 * colors,
    global float3 * normals,
    const struct Camera camera,
    const int numFrames
    ){

    local struct Resources localResources;
    localResources = *resources;

    uint x = get_global_id(0);
    uint y = get_global_id(1);

    uint index = y * localResources.width + x;
    uint seed = (numFrames<<16) ^ (numFrames >>13) + index;

    struct Sample sample = samples[index];

    if( sample.objectID < 0)
        return;

    struct Ray ray = rays[index];
    float4 lightSample = light[index];
    float3 normal = normals[index];
        
    float4 colorSample = ComputeColorSample(localResources, &ray, camera, sample, &lightSample, normal, &seed);

    rays[index] = ray;
    light[index] = lightSample;
    accumulator[index] = clamp(accumulator[index] + colorSample, 0.0f, 1.0f);
    
}
