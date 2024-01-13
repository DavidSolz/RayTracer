#include "Random.h"

float Random::Rand(uint32_t & seed){
    seed = seed *747796405u + 2891336453u;
    uint32_t word = ((seed >> ((seed >> 28u) + 4u)) ^ seed) * 277803737u;
    return ((word>>22u) ^ word)/(float)UINT_MAX;
}

float Random::UniformRandom(uint32_t & seed){
    float theta = 2 * 3.1415926f * Rand(seed);
    float rho = sqrt(-2 * log(Rand(seed)));
    return rho * cos(theta);
}
