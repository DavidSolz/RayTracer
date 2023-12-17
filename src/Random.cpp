#include "Random.h"

float Random::Rand(unsigned int& seed){
    seed = seed *747796405u + 2891336453u;
    unsigned int word = ((seed >> ((seed >> 28u) + 4u)) ^ seed) * 277803737u;
    return ((word>>22u) ^ word)/(float)UINT_MAX;
}

float Random::UniformRandom(unsigned int& seed){
    float theta = 2 * 3.1415926f * Rand(seed);
    float rho = sqrt(-2 * log(Rand(seed)));
    return rho * cos(theta);
}