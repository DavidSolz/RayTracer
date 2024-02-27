
#include "resources/kernels/KernelStructs.h"
#include "resources/kernels/ColorManipulation.h"
#include "resources/kernels/Intersections.h"


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

float4 GetColorAtPoint(
    const struct Resources resources, 
    struct Ray * ray
    ){

    global const unsigned int * textureData = resources.textureData;
    global const struct Material * materials = resources.materials;
    global const struct Object * objects = resources.objects;
    global const struct Texture * infos = resources.textureInfo;

    struct Sample sample = FindClosestIntersection(resources, ray);

    if( isinf(sample.length) )
        return 1.0f;

    struct Object object = objects[ sample.objectID ];
    struct Material material =  materials[ object.materialID ];
    struct Texture info = infos[ material.textureID ];
    float4 color = GetTexturePixel(textureData, &object, info, sample.point, sample.normal);

    return color;
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

void kernel ApplyTexture(
    write_only image2d_t image, 
    global struct Resources * resources, 
    const struct Camera camera, 
    global float4 * scratch 
    ){

    local struct Resources localResources;
    localResources = *resources;

    uint x = get_global_id(0);
    uint y = get_global_id(1);

    uint width = get_global_size(0);
    uint height = get_global_size(1);

    uint index = y * width + x;
    float3 pixelPosition = CalculatePixelPosition(x, y, width, height, &camera);

    struct Ray ray;
    ray.origin = camera.position;
    ray.direction = normalize(pixelPosition - ray.origin);

    float4 lightMapSample = scratch[ index ];
    float4 sample = GetColorAtPoint(localResources, &ray);
    
    float4 pixel = lightMapSample * sample;

    write_imagef(image, (int2)(x, y), pixel);

}
