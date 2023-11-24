#include "Random.h"



float Random::Rand(unsigned int& seed){
    seed = seed * 747796405 + 2891336453;
    unsigned int result = ((seed >>((seed>>28u)+4u)) ^ seed) * 277803737;
    result = (result>>22)^result;
    return result/4294967295.0f;
}

float Random::UniformRandom(unsigned int& seed){
    float theta = 2 * 3.1415926f * Rand(seed);
    float rho = sqrt(-2 * log(Rand(seed)));
    return rho * cos(theta);
}