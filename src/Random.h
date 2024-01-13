#ifndef RANDOM_H
#define RANDOM_H

#include <cmath>
#include <stdint.h>

class Random{
public:
static float Rand(uint32_t & seed);

static float UniformRandom(uint32_t & seed);

};


#endif
