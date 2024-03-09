#ifndef THREADEDSHADER_H
#define THREADEDSHADER_H

#include "ComputeShader.h"
#include "RenderingContext.h"
#include "Random.h"
#include "Sample.h"
#include "Ray.h"

#include <stack>
#include <thread>
#include <stdio.h>
#include <vector>
#include <cmath>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

#define EPSILON 1.0000001f
#define STACK_SIZE 32

class ThreadedShader : public ComputeShader{
private:
    
    unsigned int numThreads;
    int rowsPerThread;

    Sample ( * traverse )(RenderingContext * context, const Ray & ray);

    std::thread * threads;

    static bool AABBIntersection(const Ray & ray, const Vector3 & minimalPosition , const Vector3 & maximalPosition);

    static Sample LinearTraverse(RenderingContext * context, const Ray & ray);

    static Sample BVHTraverse(RenderingContext * context, const Ray & ray);

    Vector3 RandomDirection(unsigned int& seed);

    Color ComputeColor(struct Ray & ray, unsigned int& seed);

    void ComputeRows(const int& _startY, const int& _endY, Color * pixels);

public:

    ThreadedShader(RenderingContext * _context);

    void Render(Color * _pixels);

    ~ThreadedShader();
};


#endif