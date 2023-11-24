#ifndef THREADEDRENDERING_H
#define THREADEDRENDERING_H

#include <thread>
#include <stdio.h>
#include <vector>

#include "IFrameRender.h"
#include "RenderingContext.h"
#include "Random.h"
#include "HitInfo.h"
#include "Ray.h"

class ThreadedRendering : public IFrameRender{

    unsigned int numThreads;
    int rowsPerThread;

    std::thread *threads;

    RenderingContext *context;

    Vector3 RandomReflection(const struct Vector3& normal, unsigned int& seed);

    Vector3 RandomDirection(unsigned int& seed);

    Vector3 Reflect(const Vector3& incident, const Vector3& normal);

    float Intersect(const Ray& ray, const Sphere& sphere);
 
    HitInfo FindClosestIntersection(const struct Ray& ray);

    Color ComputeColor(struct Ray& ray, unsigned int& seed);

    void ComputeRows(int _startY, int _endY, Color* pixels);


public:

    void Init(RenderingContext * _context);

    void Render(Color * array);

    ~ThreadedRendering();

};

#endif
