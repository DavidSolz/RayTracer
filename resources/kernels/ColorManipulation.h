#ifndef COLORMANIPULATION_H
#define COLORMANIPULATION_H

#define PI 3.1415626535f
#define TWO_PI 2.0f * PI
#define ONE_OVER_MAX_CHAR 1.0f/255.0f
#define ONE_OVER_PI 1.0f/PI
#define ONE_OVER_2_PI 1.0f/TWO_PI

#include "resources/kernels/KernelStructs.h"

float4 Unpack(const unsigned int color){
    unsigned char * byte = (unsigned char *)&color;
    return (float4)(byte[0], byte[1], byte[2], byte[3]) * ONE_OVER_MAX_CHAR;
}

float4 ColorSample(
    global const unsigned int * texture, 
    const float u, 
    const float v, 
    const int width, 
    const int height, 
    const float offset
    ){

    int texelX = floor( u * (width - 1) );
    int texelY = floor( v * (height - 1) );

    int idx = offset + texelY * width + texelX;

    return Unpack(texture[ idx ]);
}

float4 GetTexturePixel(
    global const unsigned int * texture,
    const struct Object * object,
    const struct Texture info,
    const float3 localPoint,
    const float3 normal
    ){

    float u = 0.0f;
    float v = 0.0f;
    float w = 0.0f;

    float3 relativePos = localPoint - object->position;

    if( object->type == CUBE ){

        float3 boxSize = object->maxPos - object->position;
        relativePos /= boxSize;

        float conditionU = fabs(normal.x) == 1.0f;
        float conditionV = fabs(normal.y) == 1.0f;

        u = conditionU * relativePos.y + (1.0f - conditionU) * relativePos.x;
        v = ( conditionV + conditionU ) * relativePos.z + (1.0f - conditionV - conditionU) * relativePos.y;

    } else if ( object->type == DISK ){

        float theta = atan2(relativePos.z, relativePos.x);
        float radius = length( relativePos );

        u = (theta + M_PI_F) / TWO_PI;
        v = radius / object->radius;

    } else if ( object->type == PLANE){

        float width = object->maxPos.x;
        float height = object->maxPos.y;

        float3 minPos = (float3)(-width , -height, 0.0f) * 0.5f;
        float3 localPos = (relativePos - minPos);

        u = localPos.x / width;
        v = 0.5f + localPos.z / height;

    }else if ( object->type == SPHERE){
        float theta = atan2(normal.z, normal.x) + PI;
        float phi = acos(normal.y);

        u = theta * ONE_OVER_PI;
        v = phi * ONE_OVER_PI;
    }else if( object->type == TRIANGLE ){

        float3 A = object->verticeA;
        float3 B = object->verticeB;
        float3 C = object->verticeC;

        float area = fabs((B.x - A.x) * (C.y - A.y) - (B.y - A.y) * (C.x - A.x));
        float areaPBC = fabs((B.x - localPoint.x) * (C.y - localPoint.y) - (B.y - localPoint.y) * (C.x - localPoint.x));
        float areaPCA = fabs((C.x - localPoint.x) * (A.y - localPoint.y) - (C.y - localPoint.y) * (A.x - localPoint.x));
        float areaPAB = fabs((A.x - localPoint.x) * (B.y - localPoint.y) - (A.y - localPoint.y) * (B.x - localPoint.x));

        u = clamp(areaPBC / area, 0.0f, 1.0f);
        v = clamp(areaPCA / area, 0.0f, 1.0f);
        w = clamp(1.0f - u - v, 0.0f, 1.0f);

    }

    return ColorSample(texture, u, v, info.width, info.height, info.offset);
}

#endif