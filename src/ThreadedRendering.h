#ifndef THREADEDRENDERING_H
#define THREADEDRENDERING_H

#include <thread>
#include <stdio.h>
#include <vector>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

#include "IFrameRender.h"
#include "RenderingContext.h"
#include "Random.h"
#include "Sample.h"
#include "Ray.h"

class ThreadedRendering : public IFrameRender{

    unsigned int numThreads;
    int rowsPerThread;

    std::thread *threads;

    RenderingContext *context;

    Vector3 RandomReflection(const struct Vector3& normal, unsigned int& seed);

    Vector3 RandomDirection(unsigned int& seed);

    Vector3 Reflect(const Vector3& incident, const Vector3& normal);

    float IntersectSphere(const Ray &ray, const Object &object);

    float IntersectPlane(const Ray& ray, const Object& object);

    Sample FindClosestIntersection(const struct Ray& ray);

    Color ComputeColor(struct Ray& ray, unsigned int& seed);

    void ComputeRows(const int& _startY, const int& _endY, Color* pixels);


public:

    void Init(RenderingContext * _context);

    void Render(Color * _pixels);

    ~ThreadedRendering();

};

#endif
