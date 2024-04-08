
#include "resources/kernels/KernelStructs.h"
#include "resources/kernels/ColorManipulation.h"


#define ALPHA_MIN 0.001f
#define INPUT_IOR 1.0f

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

void QSwap(int *a, int *b){
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
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

    float cosI = dot(-incident, normal);
    float sinR2 = (1.0f - cosI * cosI);

    float eta = n1/n2;

    if( eta * sinR2 > 1.0f)
        return 0.0f;//Reflect(incident, -normal);

    float cosR2 = sqrt(1.0f - sinR2 * sinR2);
    float frag1 = mad(eta, cosI, -cosR2);

    float3 direction = mad(incident, eta, normal * frag1);
    return normalize(direction);
}

float SchlickFresnel(const float value){
    float temp = 1.0f - value;
    return temp * temp * temp * temp * temp;
}

float4 DiffuseBRDF(const float cosView, const float cosLight, const struct Material material){

    float FL = SchlickFresnel(cosLight);
    float FV = SchlickFresnel(cosView);

    float R = 0.5f + 2.0f * cosLight * cosLight * material.roughness * material.roughness;
    float retro = R * ( FL + FV + FL * FV * (R - 1.0f) );

    return ONE_OVER_PI * ( (1.0f - 0.5f * FL) * (1.0f - 0.5f * FV) + retro );
}

float GgxAnisotropic(const float3 halfVector, const float ax, const float ay){

    float dotHX2 = halfVector.x * halfVector.x;
    float dotHY2 = halfVector.z * halfVector.z;
    float cos2Theta = cos(halfVector.y) * cos(halfVector.y);
    float ax2 = ax * ax;
    float ay2 = ay * ay;

    float temp = (dotHX2 / ax2 + dotHY2 / ay2 + cos2Theta);

    return ONE_OVER_PI * 1.0f / (ax * ay * temp * temp);
}

float SeparableSmithGGXG1BSDF(const float3 vector, const float3 halfVector, const float ax, const float ay){

    float cos2Theta = halfVector.y * halfVector.y;
    float sin2Theta = 1.0f - cos2Theta;

    float tanTheta = sqrt( sin2Theta/cos2Theta );

    float cos2Phi = vector.x * vector.x;
    float sin2Phi = 1.0f - cos2Phi;

    float a = sqrt( cos2Phi * ax * ax + sin2Phi * ay * ay);
    float a2Tan2Theta = a * a * tanTheta * tanTheta;

    float lambda = 0.5f * (-1.0f + sqrt( 1.0f + a2Tan2Theta ));
    return 1.0f / (1.0f + lambda);

}

float4 SpecularBSDF(const float3 normal, const float3 lightVector, const float3 viewVector, const float3 halfVector, const struct Material material){

    float aspect = sqrt(1.0f - 0.9f * material.anisotropy);

    float roughnessSqr = material.roughness * material.roughness;

    float ax = fmax(ALPHA_MIN, roughnessSqr / aspect);
    float ay = fmax(ALPHA_MIN, roughnessSqr * aspect);

    float cosLight = dot(normal, lightVector);
    float cosView = dot(normal, viewVector);

    float D = GgxAnisotropic(halfVector, ax, ay);
    float Gl = SeparableSmithGGXG1BSDF(lightVector, halfVector, ax, ay);
    float Gv = SeparableSmithGGXG1BSDF(viewVector, halfVector, ax, ay);

    return D * Gl * Gv / (4.0f * cosLight * cosView);
}

float4 SpecularTransmissionBSDF(const float3 lightVector, const float3 viewVector, const float3 halfVector, const struct Material material){
    float aspect = sqrt(1.0f - 0.9f * material.anisotropy);

    float roughnessSqr = material.roughness * material.roughness;

    float ax = fmax(ALPHA_MIN, roughnessSqr / aspect);
    float ay = fmax(ALPHA_MIN, roughnessSqr * aspect);

    float cosViewHalf = dot(viewVector, halfVector);

    cosViewHalf *= halfVector.y;

    float eta = 1.0f / material.indexOfRefraction;

    float D = GgxAnisotropic(halfVector, ax, ay);
    float Gl = SeparableSmithGGXG1BSDF(lightVector, halfVector, ax, ay);
    float Gv = SeparableSmithGGXG1BSDF(viewVector, halfVector, ax, ay);
    float F = eta + (1.0f - eta) * SchlickFresnel(cosViewHalf);

    return D * F * Gl * Gv;
}

float4 Tint(const float4 albedo){
    float luminance = albedo.x * 0.3f + albedo.y * 0.6f + albedo.z;
    float condition = luminance > 0.0f;
    return mix(1.0f, albedo/luminance , condition);
}

float4 Sheen(const float cosLightHalf, const struct Material material){
    float4 tint = Tint(material.albedo);
    float4 sheen = mix(1.0f, tint, material.tintRoughness);
    return sheen * SchlickFresnel(cosLightHalf) * material.tintWeight;
}

float GTR(const float cosLightHalf, const float alpha){

    if( alpha >= 1.0f)
        return ONE_OVER_PI;

    float alphaSqr = alpha * alpha;
    float decAlphaSqr = alphaSqr - 1.0f;

    return ONE_OVER_PI * decAlphaSqr/( log2(alphaSqr)*(1.0f + decAlphaSqr * cosLightHalf * cosLightHalf) );
}

float SeparableSmithGGXG1(const float cosine, float alpha){
    float a2 = alpha * alpha;
    return 2.0f / (1.0f + sqrt(a2 + (1 - a2) * cosine * cosine));
}

float4 ClearcoatBRDF(const float3 viewVector, const float3 lightVector, const float3 halfVector, const struct Material material) {

    float cosHalf = fabs(halfVector.y);
    float cosView = fabs(viewVector.y);
    float cosLight = fabs(lightVector.y);
    float cosLightHalf = dot(lightVector, halfVector);

    float scale = mix(0.1f, 0.001f, material.clearcoatRoughness);

    float D = GTR(cosHalf, scale);
    float Gl = SeparableSmithGGXG1(cosLight, 0.25f);
    float Gv = SeparableSmithGGXG1(cosView, 0.25f);
    float F = 0.04f + 0.96f * SchlickFresnel(cosLightHalf);

    return 0.25f * D * Gl * Gv * F;
}

float4 CalculateWeights(const struct Material material){
    float metallic = material.metallic;
    float transmission = (1.0f - material.metallic) * material.transparency;
    float dielectric = (1.0f - material.metallic) * (1.0f - material.transparency);

    float4 weights;

    weights.x = metallic + dielectric; //specular
    weights.y = transmission; // transmission
    weights.z = dielectric; // diffuse
    weights.w = material.clearcoatThickness; // clearcoat

    return normalize(weights);
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

    struct Object object = objects[ sample.objectID ];
    struct Material material =  materials[ object.materialID ];
    struct Texture info = infos[ material.textureID ];

    float3 lightVector = normalize(-ray->direction);
    float3 viewVector = normalize(camera.position - sample.point);
    float3 halfVector = normalize(lightVector + viewVector);

    float3 diffusionDirection = DiffuseReflect(normal, seed);
    float3 reflectionDirection = Reflect(ray->direction, normal);
    float3 refractionDirecton = Refract(viewVector, normal, INPUT_IOR, material.indexOfRefraction);

    float3 outgoing = mix(diffusionDirection, reflectionDirection, material.metallic);

    ray->origin = sample.point;
    ray->direction = normalize( mix(outgoing, refractionDirecton, material.transparency) );

    float cosLight = fmax(1e-6f, dot(normal, lightVector));
    float cosView = fmax(1e-6f, dot(normal, viewVector));
    float cosLightHalf = fmax(1e-6f, dot(lightVector, halfVector));

    float4 emission = material.albedo * material.emmissionIntensity;
    float isEmissive = dot(emission.xyz, (float3)(1.0f, 1.0f, 1.0f)) > 0.0f;

    float4 texture = GetTexturePixel(textureData, &object, info, sample.point, normal);

    float4 diffuseAlbedo = (1.0f - material.metallic) * material.diffuse * texture;
    float4 specularAlbedo = mix(material.specular, 1.0f, material.metallic);
    float4 fresnel = SchlickFresnel(cosLightHalf);

    float4 diffuseComponent = diffuseAlbedo * (1.0f - fresnel ) * DiffuseBRDF(cosView, cosLight, material);
    float4 specularComponent = specularAlbedo * fresnel * SpecularBSDF(normal, lightVector, viewVector, halfVector, material);
    float4 transmissionComponent = SpecularTransmissionBSDF(lightVector, viewVector, halfVector, material);
    float4 clearcoatComponent = ClearcoatBRDF(viewVector, lightVector, halfVector, material);
    float4 sheenComponent = Sheen( cosLightHalf, material);

    float4 weights = CalculateWeights(material);

    float4 colorSample = emission * isEmissive;
    colorSample += (diffuseComponent + sheenComponent) * weights.z;
    colorSample += clearcoatComponent * weights.w;
    colorSample += specularComponent * weights.x;
    colorSample += transmissionComponent * weights.y;
    colorSample *=  *lightSample * (cosLight > 0.0f);

    (*lightSample) *= texture * material.albedo * 2.0f * cosLight;

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
    struct Ray ray = rays[index];
    float4 lightSample = light[index];
    float3 normal = normals[index];

    if( sample.objectID < 0){

        const struct Texture info = localResources.textureInfo[1];
        const global unsigned int * textureData = localResources.textureData;

        float u = ( atan2(ray.direction.x, ray.direction.z) + PI ) * ONE_OVER_PI;
        float v = acos(-ray.direction.y) * ONE_OVER_PI;

        float4 texel = ColorSample(textureData, u, v, info.width, info.height, info.offset);

        accumulator[index] += texel * lightSample * 0.25f;
        return;
    }


    float4 colorSample = ComputeColorSample(localResources, &ray, camera, sample, &lightSample, normal, &seed);

    rays[index] = ray;
    light[index] = clamp(lightSample, 0.0f, 1.0f);
    accumulator[index] = clamp(accumulator[index] + colorSample, 0.0f, 1.0f);

}
