#ifndef THREADEDSHADER_H
#define THREADEDSHADER_H

#include "ComputeShader.h"
#include "RenderingContext.h"
#include "Random.h"
#include "Sample.h"
#include "Ray.h"

#include <thread>
#include <stdio.h>
#include <vector>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

class ThreadedShader : public ComputeShader{
private:
    
    unsigned int numThreads;
    int rowsPerThread;

    std::thread *threads;

    bool CheckForHyperthreading();

public:

    ThreadedShader(RenderingContext * _context);

    void Render(Color * _pixels);

    ~ThreadedShader();
};


#endif