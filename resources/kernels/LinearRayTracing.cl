
#include "resources/kernels/KernelStructs.h"
#include "resources/kernels/ColorManipulation.h"
#include "resources/kernels/Intersections.h"


#define NUM_BOUNCES 8

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

void ComputeBoxNormal(struct Sample * sample,const struct Object * object){

    float3 boxCenter = (object->maxPos + object->position) * 0.5f;
    float3 radius = (object->maxPos - object->position) * 0.5f;
    float3 pointToCenter = sample->point - boxCenter;

    sample->normal = normalize(sign(pointToCenter) * step(fabs(fabs(pointToCenter) - radius), EPSILON));
}

struct Sample FindClosestIntersection(const struct Resources resources, const struct Ray * ray){

    global const struct Object * objects = resources.objects;
    int numObject = resources.numObject;

    struct Sample sample = {0};
    sample.length = INFINITY;

    for (int id = 0; id < numObject; ++id) {

        struct Object object = objects[id];

        float length = -1.0f;

        switch(object.type){
            case CUBE:
                length = IntersectCube(ray, &object);
                break;
            case PLANE:
                length = IntersectPlane(ray, &object);
                break;
            case DISK:
                length = IntersectDisk(ray, &object);
                break;
            case TRIANGLE:
                length = IntersectTriangle(ray, &object);
                break;
            default:
                length = IntersectSphere(ray, &object);
        }

        if( (length < sample.length) && (length > 0.01f) ){
            sample.length = length ;
            sample.point = ray->origin + ray->direction * length * EPSILON;
            sample.objectID = id;

            switch(object.type){

                case CUBE:
                    ComputeBoxNormal(&sample, &object);
                    break;

                case SPHERE:
                    sample.normal = normalize(sample.point - object.position);
                    break;

                default :
                    sample.normal = normalize(object.normal);
                    break;
            }

        }

    }

    return sample;
}

float3 Reflect(const float3 incoming, const float3 normal) {

    float3 outgoing = incoming - normal * 2.0f * dot(incoming, normal);
    return normalize(outgoing);

}

float3 DiffuseReflect(const float3 normal, uint * seed){

    float3 randomDirection = RandomDirection(seed);
    float cosDirection = dot(normal, randomDirection);

    return normalize(randomDirection * cosDirection + normal );

}

float3 Refract(
    const float3 incoming, 
    float3 normal, 
    const float n1, 
    const float n2
    ){

    float cosI = -dot(incoming, normal);
    float sinI = sqrt(1.0f - cosI*cosI);
    float eta = n1/n2;

    if( eta * sinI > 1.0f)
        return Reflect(incoming, normal);

    float3 perpendicularRay =  eta * (incoming + cosI * normal);
    float rayLength = length(perpendicularRay);
    float3 parallelRay = -sqrt(1.0f - rayLength*rayLength) * normal;

    return normalize(perpendicularRay + parallelRay);
}

float IORToR0(const float indexOfRefraction){
    float R0 = (1.0f - indexOfRefraction) / (1.0f + indexOfRefraction);
    return R0 * R0;
}

float4 SchlickFresnel(const float cosHalf, const float4 F0){
    return F0 + (1.0f - F0) * pow(1.0f - cosHalf, 5.0f);
}

float GGX(const float cosHalf, const float roughness){
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float cosHalf4 = cosHalf*cosHalf*cosHalf*cosHalf;
    return ONE_OVER_PI * alpha2 / (cosHalf4 * (alpha2 - 1.0f) + 1.0f);
}

float SchlickGGX(const float cosDirection, const float roughness){
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;

    float nominator = 2.0f * cosDirection;
    float denominator = cosDirection + sqrt(alpha2 + (1.0f - alpha2) * cosDirection * cosDirection);

    return fmin(nominator / denominator, 1.0f);
}

float GeometricSmithShlickGGX(
    const float cosView, 
    const float cosLight, 
    const float roughness
    ){

    float ggx1 = SchlickGGX(cosLight, roughness);
    float ggx2 = SchlickGGX(cosView, roughness);

    return ggx1*ggx2;
}

float4 PhysicalBRDF(
    const float3 normal, 
    const float3 halfVector, 
    const float3 lightVector, 
    const float3 outgoing, 
    const float3 viewVector, 
    const float4 attenuation,
    struct Material * material
    ){

    float cosHalf = clamp(dot(normal, halfVector), 0.0f, 1.0f);
    float cosLight = clamp(dot(normal, lightVector), 0.0f, 1.0f);
    float gloss = 1.0f - material->roughness;

    float diffuse =  2.0f * cosLight ;
    float4 specular = cosHalf * (diffuse>0.0f);
    float exponent = exp2(gloss) + 2.0f;
    specular *= pow( specular, exponent) * gloss * attenuation * material->albedo;

    return specular + diffuse;//(D * F * G) / denominator;

}

float4 ComputeLightMap(
    const struct Resources resources, 
    struct Ray * ray, 
    const struct Camera * camera, 
    uint * seed
    ){

    float4 accumulatedColor = 0.0f;
    float4 lightColor = 1.0f;
    float lastIOR = 1.0f;

    const float4 skyColor = (float4)(0.2f, 0.2f, 0.25f, 1.0f);

    global const unsigned int * textureData = resources.textureData;
    global const struct Material * materials = resources.materials;
    global const struct Object * objects = resources.objects;
    global const struct Texture * infos = resources.textureInfo;

    for(int i = 0; i < NUM_BOUNCES; ++i){
        struct Sample sample = FindClosestIntersection(resources, ray);

        if( isinf(sample.length) ){
            accumulatedColor += skyColor * lightColor;
            break;
        }

        ray->origin = sample.point;

        struct Object object = objects[ sample.objectID ];
        struct Material material =  materials[ object.materialID ];
        struct Texture info = infos[ material.textureID ];

        float3 normal = sample.normal;

        float3 lightVector = normalize(-ray->direction);
        float3 viewVector = normalize(camera->position - sample.point);

        float3 diffusionDirection = DiffuseReflect(normal, seed);
        float3 reflectionDirection = Reflect(ray->direction, normal);
        float3 refractionDirecton = Refract(ray->direction, normal, lastIOR, material.indexOfRefraction);

        float3 halfVector = normalize(normal + reflectionDirection);

        float cosLight = dot(normal, lightVector);
        float cosView = dot(normal, viewVector);

        float m = 2.0f * sqrt( pow( reflectionDirection.x, 2.0f ) + pow( reflectionDirection.y, 2.0f ) + pow( reflectionDirection.z + 1.0f, 2.0f ) );
        float2 n = reflectionDirection.xy/m + 0.5f;

        float isSpecular = Rand(seed) >=  material.roughness;

        float3 direction = mix(diffusionDirection, reflectionDirection, material.metallic * isSpecular);
        ray->direction = normalize(mix(direction, refractionDirecton, material.transparency * (material.metallic==0.0f)));

        float4 emission =  material.albedo * material.emmissionIntensity * (cosLight> 0.0f);

        accumulatedColor += emission * lightColor; 

        lightColor *= 2.0f * cosLight * material.albedo;
        lastIOR = material.indexOfRefraction;
    }

    return accumulatedColor;
}

float3 CalculatePixelPosition(
    const int x,
    const int y,
    const int width,
    const int height,
    const struct Camera * camera
    ){

    float tanHalfFOV = tan(radians(camera->fov) * 0.5f);
    float pixelXPos = (2.0 * x / width - 1.0f) * camera->aspectRatio  ;
    float pixelYPos = (2.0 * y / height - 1.0f);

    return camera->position + (camera->front + ( camera->right * pixelXPos + camera->up * pixelYPos) * tanHalfFOV ) * camera->near;
}



// Main

void kernel ComputeLight(
    global struct Resources * resources, 
    const struct Camera camera, 
    const int numFrames,
    global float4 * scratch 
    ){

    local struct Resources localResources;
    localResources = *resources;

    uint x = get_global_id(0);
    uint y = get_global_id(1);

    uint width = get_global_size(0);
    uint height = get_global_size(1);

    uint index = y * width + x;
    uint seed = (numFrames<<16) ^ (numFrames >>13) + index;

    float3 offset = RandomDirection(&seed);
    float3 pixelPosition = CalculatePixelPosition(x + offset.x + 0.5f, y + offset.y + 0.5f, width, height, &camera);

    struct Ray ray;
    ray.origin = camera.position;
    ray.direction = normalize(pixelPosition - ray.origin);

    float4 ligthSample = ComputeLightMap(localResources, &ray, &camera, &seed);

    float scale = 1.0f/(1+ numFrames);
    scratch[index] = mix(scratch[index], ligthSample, scale);

}
