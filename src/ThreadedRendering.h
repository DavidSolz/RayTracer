#ifndef THREADEDRENDERING_H
#define THREADEDRENDERING_H

#include <thread>
#include <stdio.h>
#include <vector>

#include "IFrameRender.h"
#include "RenderingContext.h"

class ThreadedRendering : public IFrameRender{

    unsigned int numThreads;
    int rowsPerThread;

    std::thread *threads;

    RenderingContext *context;

    Vector3 Reflect(const Vector3& incident, const Vector3& normal);

    float Intersect(const Vector3& rayOrigin, const Vector3& rayDirection, const Sphere& sphere);

    Color ComputeColor(const Vector3& intersectionPoint, const Vector3& cameraPos, const Sphere &sphere, const Vector3& sunPosition);

    void ComputeRows(int _startY, int _endY, Color* pixels);


public:

    void Init(RenderingContext * _context);

    void Render(Color * array);

    ~ThreadedRendering();

};

#endif
