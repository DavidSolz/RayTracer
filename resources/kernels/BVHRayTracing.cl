
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


void ComputeBoxNormal(struct Sample * sample,const struct Object * object){

    float3 boxCenter = (object->maxPos + object->position) * 0.5f;
    float3 radius = (object->maxPos - object->position) * 0.5f;
    float3 pointToCenter = sample->point - boxCenter;

    sample->normal = normalize(sign(pointToCenter) * step(fabs(fabs(pointToCenter) - radius), EPSILON));
}

struct Sample FindClosestIntersection(const struct Resources resources, const struct Ray * ray){

    global const struct BoundingBox * boxes = resources.boxes;

    global const struct Object * objects = resources.objects;
    int numObject = resources.numObject;

    struct Sample sample = {0};
    sample.length = INFINITY;

    int stack[STACK_SIZE];
    int top = 0;

    float3 scaledDir = ray->direction * EPSILON;

    stack[top++] = 0;

    while (top > 0) {
        
        int boxID = stack[--top];
        struct BoundingBox box = boxes[boxID];

        int leftChildIndex = box.leftID;
        int rightChildIndex = box.rightID;

        if ( box.objectID != -1 ) {

            struct Object object = objects[ box.objectID ];

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
                sample.point = ray->origin + scaledDir * length ;
                sample.objectID = box.objectID;

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

        } else {

            if( leftChildIndex > 0){
                struct BoundingBox left = boxes[leftChildIndex];

                if( AABBIntersection(ray, left.minimalPosition, left.maximalPosition) )
                    stack[top++] = leftChildIndex;
            }
                
            if( rightChildIndex > 0){
                struct BoundingBox right = boxes[rightChildIndex];

                if( AABBIntersection(ray, right.minimalPosition, right.maximalPosition) )
                    stack[top++] = rightChildIndex;
            }

        }
    }

    return sample;
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
        return 0.0f;

    float3 perpendicularRay =  eta * incident;
    float3 parallelRay = eta * cosI - sqrt(fmax(0.0f, 1.0f - cosT)) * normal;

    return normalize(perpendicularRay + parallelRay);
}

float IORToR0(const float indexOfRefraction){
    float R0 = (1.0f - indexOfRefraction) / (1.0f + indexOfRefraction);
    return R0 * R0;
}

float4 SchlickFresnel(const float cosTheta, const float4 F0){
    return F0 + (1.0f - F0) * pow(1.0f - cosTheta, 5.0f);
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
    struct Material * material
    ){

    float cosIncident = clamp(-dot(viewVector, halfVector), 0.0f, 1.0f);
    float cosHalf = dot(normal, halfVector);

    float F0 = (material->indexOfRefraction-1.0f)/(material->indexOfRefraction+1.0f);
    float4 fresnel = SchlickFresnel(cosIncident, F0 * F0);

    float alpha = material->roughness + 2;
    float alphaSqr = alpha * alpha;

    float specular = ONE_OVER_PI * alphaSqr * exp(-alphaSqr / (2.0f * material->roughness)) * 1.0f/cosHalf;

    float4 reflection = fresnel * specular;
    float4 refraction = (1.0f - fresnel) * (1.0f - specular);

    float4 final_color = (reflection  + refraction) * material->albedo;

    return final_color;
}

float4 ComputeLightMap(
    const struct Resources resources, 
    struct Ray * ray, 
    const struct Camera * camera, 
    uint * seed
    ){

    float4 accumulatedLight = 0.0f;
    float4 lightColor = 1.0f;
    float lastIOR = 1.0f;

    global const unsigned int * textureData = resources.textureData;
    global const struct Material * materials = resources.materials;
    global const struct Object * objects = resources.objects;
    global const struct Texture * infos = resources.textureInfo;

    const float4 skyColor = (float4)(0.2f, 0.2f, 0.25f, 1.0f);

    for(int i = 0; i < NUM_BOUNCES; ++i){
        struct Sample sample = FindClosestIntersection(resources, ray);

        if( isinf(sample.length) ){
            accumulatedLight += skyColor * lightColor;
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

        float isMetallic = (Rand(seed) < material.metallic);

        float3 direction = mix(diffusionDirection, reflectionDirection, material.roughness * isMetallic);
        ray->direction = normalize( mix(direction, refractionDirecton, material.transparency) );

        float4 emission =  material.albedo * material.emmissionIntensity * (cosLight> 0.0f);
        float4 pbr = PhysicalBRDF(normal, halfVector, lightVector, ray->direction, viewVector, &material);

        accumulatedLight += (emission) * lightColor; 

        lightColor *= 2.0f * cosLight * material.albedo;
    }

    return accumulatedLight;
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
