#include "resources/kernels/KernelStructs.h"
#include "resources/kernels/Intersections.h"

#define STACK_SIZE 64

kernel void Traverse(
    global struct Resources * resources,
    global struct Ray * rays,
    global struct Sample * samples,
    global float3 * normals
    ){

    local struct Resources localResources;

    localResources = *resources;

    global const struct BoundingBox * boxes = localResources.boxes;
    global const struct Object * objects = localResources.objects;

    int gx = get_global_id(0);
    int gy = get_global_id(1);

    int width = get_global_size(0);

    int globalIndex = gy * width + gx;

    int lx = get_local_id(0);
    int ly = get_local_id(1);

    int lwidth = get_local_size(0);

    int localIndex = ly * lwidth + lx;

    struct Ray ray = rays[globalIndex];

    struct Sample sample = {0};
    sample.objectID = -1;

    float minLength = INFINITY;
    float length = -1.0f;

    int stack[ STACK_SIZE ] = {};
    int top = 0;

    stack[top++] = 0;

    while ( top > 0 ) {

        int boxID = stack[--top];
        struct BoundingBox box = boxes[ boxID ];

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
                sample.point = ray.origin + ray.direction * length;
                sample.objectID = box.objectID;

            }

        } else {

            if( leftChildIndex > 0){
                struct BoundingBox left = boxes[ leftChildIndex ];

                if( AABBIntersection(&ray, left.minimalPosition, left.maximalPosition) )
                    stack[top++] = leftChildIndex;

            }

            if( rightChildIndex > 0){

                struct BoundingBox right = boxes[ rightChildIndex ];

                if( AABBIntersection(&ray, right.minimalPosition, right.maximalPosition) )
                    stack[top++] = rightChildIndex;
            }

        }

    }

    samples[globalIndex] = sample;

    if( sample.objectID == -1 )
        return;

    struct Object object = objects[ sample.objectID ];

    if ( object.type == SPHERE){

        normals[globalIndex] = normalize( sample.point - object.position);

    }else if( object.type == TRIANGLE ){

        float3 A = object.verticeA;
        float3 B = object.verticeB;
        float3 C = object.verticeC;

        float3 v0 = (B - A);
        float3 v1 = (C - A);
        float3 v2 = sample.point - A;

        float dot00 = dot(v0, v0);
        float dot01 = dot(v0, v1);
        float dot02 = dot(v0, v2);
        float dot11 = dot(v1, v1);
        float dot12 = dot(v1, v2);

        float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
        float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
        float v = (dot00 * dot12 - dot01 * dot02) * invDenom;
        float w = 1.0f - u - v;

        normals[globalIndex] = normalize(object.normalA * w + object.normalB * u + object.normalC * v);
    }
}
