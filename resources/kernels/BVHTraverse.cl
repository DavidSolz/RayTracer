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

    int stack[ STACK_SIZE ];
    int top = 0;

    float3 scaledDir = ray.direction * EPSILON;

    stack[top++] = 0;

    while ( top > 0 ) {

        int boxID = stack[--top];
        struct BoundingBox box = boxes[boxID];

        int leftChildIndex = box.leftID;
        int rightChildIndex = box.rightID;

        if ( box.objectID >= 0 ) {

            struct Object object = objects[ box.objectID ];

            if ( object.type == TRIANGLE ){
                length = IntersectTriangle(&ray, &object);
            }else{
                length = IntersectSphere(&ray, &object);
            }

            if( (length < minLength) && (length > 0.01f) ){

                minLength = length;
                sample.point = ray.origin + scaledDir * length;
                sample.objectID = box.objectID;

            }

        } else {

            if( leftChildIndex > 0){
                struct BoundingBox left = boxes[leftChildIndex];

                if( AABBIntersection(&ray, left.minimalPosition, left.maximalPosition) ){
                    stack[top++] = leftChildIndex;
                }
            }

            if( rightChildIndex > 0){
                struct BoundingBox right = boxes[rightChildIndex];

                if( AABBIntersection(&ray, right.minimalPosition, right.maximalPosition) )
                    stack[top++] = rightChildIndex;
            }

        }

    }

    samples[index] = sample;
}
