#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include "Vector3.h"
#include <stdint.h>

struct BoundingBox{
    int32_t objectID;

    int32_t parentID;
    int32_t leftID;
    int32_t rightID;

    Vector3 minimalPosition;
    Vector3 maximalPosition;

    BoundingBox(){
        this->minimalPosition = Vector3(INFINITY, INFINITY, INFINITY);
        this->maximalPosition = Vector3(-INFINITY, -INFINITY, -INFINITY);
        this->objectID = -1;
        this->leftID = -1;
        this->rightID = -1;
    }

    void Expand(const BoundingBox& other) {

        minimalPosition.x = std::fmin(minimalPosition.x, other.minimalPosition.x);
        minimalPosition.y = std::fmin(minimalPosition.y, other.minimalPosition.y);
        minimalPosition.z = std::fmin(minimalPosition.z, other.minimalPosition.z);

        maximalPosition.x = std::fmax(maximalPosition.x, other.maximalPosition.x);
        maximalPosition.y = std::fmax(maximalPosition.y, other.maximalPosition.y);
        maximalPosition.z = std::fmax(maximalPosition.z, other.maximalPosition.z);
    }

    float SurfaceArea(){

        Vector3 deltas = maximalPosition - minimalPosition;

        float sideA = deltas.x * deltas.y;
        float sideB = deltas.x * deltas.z;
        float sideC = deltas.y * deltas.z;

        return 2.0f * (sideA + sideB + sideC);

    }

} __attribute((aligned(64)));


#endif
