#include "resources/kernels/KernelStructs.h"
#include "resources/kernels/Intersections.h"

#define STACK_SIZE 32

kernel void Traverse(
    global struct Resources * resources,
    global struct Ray * rays,
    global struct Sample * samples
    ){

    local struct Resources localResources;
    localResources = *resources;

    uint x = get_global_id(0);
    uint y = get_global_id(1);

    uint width = get_global_size(0);
    uint height = get_global_size(1);

    uint index = y * width + x;

    struct Ray ray = rays[index];

    global const struct BoundingBox * boxes = localResources.boxes;

    global const struct Object * objects = localResources.objects;
    int numObject = localResources.numObject;

    struct Sample sample = {0};
    sample.objectID = -1;
    float minLength = INFINITY;
    float length = -1.0f;

    float3 scaledDir = ray.direction * EPSILON;

    for (int id = 0; id < numObject; ++id) {

        struct Object object = objects[id];

        if ( object.type == TRIANGLE ){
            length = IntersectTriangle(&ray, &object);
        }else{
            length = IntersectSphere(&ray, &object);
        }

        if( (length < minLength) && (length > 0.01f) ){

            minLength = length ;
            sample.point = ray.origin + scaledDir * length ;
            sample.objectID = id;

        }

    }

    samples[index] = sample;
}
