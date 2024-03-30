#ifndef INTERSECTIONS_H
#define INTERSECTIONS_H

#include "resources/kernels/KernelStructs.h"

#define EPSILON 1.000001f

#define PI 3.1415626535f
#define TWO_PI 2.0f * PI
#define ONE_OVER_MAX_CHAR 1.0f/255.0f
#define ONE_OVER_PI 1.0f/PI
#define ONE_OVER_2_PI 1.0f/TWO_PI


float IntersectSphere(const struct Ray * ray, const struct Object * object) {
    float3 originToSphereCenter = ray->origin - object->position;

    float b = dot(originToSphereCenter, ray->direction);
    float c = dot(originToSphereCenter, originToSphereCenter) - object->radius * object->radius;
    float delta = b * b - c;

    float sqrtDelta =  sqrt(delta);
    float t1 = (-b - sqrtDelta);
    float t2 = (-b + sqrtDelta);

    return fmin(t1, t2);
}

float IntersectTriangle(const struct Ray * ray, const struct Object * object) {

    float3 A = object->verticeA;
    float3 B = object->verticeB;
    float3 C = object->verticeC;

    float3 e1 = (B - A);
    float3 e2 = (C - A);

    float3 axis = cross(ray->direction, e2);
    float det = dot(e1, axis);

    if( fabs(det) < 1e-6f) 
        return -1.0f;

    float inverseDeterminant = 1.0f / det;
    float3 rayToTriangle = ray->origin - A;
    float u = inverseDeterminant * dot(rayToTriangle, axis);

    if( (u < 0.0f) || (u >1.0f) )
        return -1.0f;

    float3 q = cross(rayToTriangle, e1);
    float v = inverseDeterminant * dot(ray->direction, q);

    if( (v < 0.0f) || (u+v >1.0f) )
        return -1.0f;

    return inverseDeterminant * dot(e2, q);

}

bool AABBIntersection(const struct Ray * ray, const float3 minimalPosition , const float3 maximalPosition){

    float3 invDirection = 1.0f / ray->direction;

    float3 tMin = (minimalPosition - ray->origin) * invDirection;
    float3 tMax = (maximalPosition - ray->origin) * invDirection;

    float3 t1 = fmin(tMin, tMax);
    float3 t2 = fmax(tMin, tMax);

    float tNear = fmax(t1.x, fmax(t1.y, t1.z));
    float tFar = fmin(t2.x, fmin(t2.y, t2.z));

    return tNear <= tFar && tFar >= 0.0f;
}

#endif
