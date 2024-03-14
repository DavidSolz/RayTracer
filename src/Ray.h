#ifndef RAY_H
#define RAY_H

#include "Vector3.h"

struct Ray {
    Vector3 origin;
    Vector3 direction;
} __attribute__((aligned(32)));

#endif