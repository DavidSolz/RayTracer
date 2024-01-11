#ifndef RANDOM_H
#define RANDOM_H

#include <cmath>
#include <stdint.h>

class Random{
public:
static float Rand(unsigned int& seed);

static float UniformRandom(unsigned int& seed);

};


#endif
