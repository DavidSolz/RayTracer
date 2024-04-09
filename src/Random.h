#ifndef RANDOM_H
#define RANDOM_H

#define PI_HALF 3.1415926535f * 0.5f;
#define TWO_PI 2.0f * 3.1415926535f;

#include <cmath>
#include <stdint.h>

namespace Random{

float Rand(uint32_t & seed);

float UniformRandom(uint32_t & seed);

};


#endif
