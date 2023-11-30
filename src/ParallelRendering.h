#ifndef PARALLELRENDERING_H
#define PARALLELRENDERING_H

#define CL_HPP_TARGET_OPENCL_VERSION 200
#define CL_HPP_ENABLE_EXCEPTIONS

#ifdef __APPLE__

#include <GL/glew.h>

#include <OpenCL/opencl.h>
#include <OpenCL/cl_gl.h>
#include "../OpenCL/include/CL/cl.hpp"

#elif __WIN32__

#include <windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <CL/opencl.hpp>
#include <CL/cl_gl.h>

#endif

#include <fstream>

#include "IFrameRender.h"

class ParallelRendering : public IFrameRender {

    RenderingContext * context;

    cl::Device default_device;
    cl::CommandQueue queue;
    cl::Context deviceContext;
    cl::Kernel kernel;

    cl::NDRange globalRange;
    cl::NDRange localRange;

    size_t dataSize;
    size_t objectBufferSize;

    cl::Buffer pixelBuffer;
    cl::Buffer frameCounter;
    cl::Buffer objectBuffer;
    cl::Buffer objectsCountBuffer;
    cl::Buffer cameraBuffer;


    cl::Device GetDefaultCLDevice();
    cl::Program FetchProgram();

public:

    void Init(RenderingContext * _context);

    void Render(Color * _pixels);

    ~ParallelRendering();

};
#endif
