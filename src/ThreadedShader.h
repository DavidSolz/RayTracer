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
#define ALPHA_MIN 0.001f
#define INPUT_IOR 1.0f

class ThreadedShader : public ComputeShader{
private:

    unsigned int numThreads;
    int rowsPerThread;

    Sample ( * traverse )(RenderingContext * context, const Ray & ray);

    std::thread * threads;

    static bool AABBIntersection(const Ray & ray, const Vector3 & minimalPosition , const Vector3 & maximalPosition);

    Vector3 DiffuseReflect(const struct Vector3& normal, unsigned int& seed);

    Vector3 Reflect(const Vector3& incident, const Vector3& normal);

    Vector3 Refract(const Vector3& incident, const Vector3& normal, const float & n1, const float & n2);

    Vector3 CalculateWeights(const Material & material);

    static Sample LinearTraverse(RenderingContext * context, const Ray & ray);

    static Sample BVHTraverse(RenderingContext * context, const Ray & ray);

    static float IntersectTriangle(const Ray & ray, const Object & object, float & u, float & v);

    static float IntersectSphere(const Ray &ray, const Object &object);

    Vector3 RandomDirection(unsigned int& seed);

    Color ComputeColor(Ray & ray, const Sample & sample, Color & lightSample, unsigned int& seed);

    void ComputeRows(const int& _startY, const int& _endY, Color * pixels);

public:

    ThreadedShader(RenderingContext * _context);

    void Render(Color * _pixels);

    ~ThreadedShader();
};


#endif
