#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include "Vector3.h"
#include <stdint.h>

struct BoundingBox{
    int32_t objectID;

    int32_t leftID;
    int32_t rightID;

    Vector3 minimalPosition;
    Vector3 maximalPosition;
};


#endif