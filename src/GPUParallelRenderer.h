#ifndef GPUPARALLELRENDERER_H
#define GPUPARALLELRENDERER_H

#define CL_HPP_TARGET_OPENCL_VERSION 200
#define CL_HPP_ENABLE_EXCEPTIONS

#ifdef __APPLE__

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <OpenGL/gl3.h>
#include <OpenGL/OpenGL.h>
#include <OpenCL/opencl.h>
#include <OpenCL/cl_gl.h>
#include "../OpenCL/include/CL/cl.hpp"

#elif __WIN32__

typedef unsigned int uint;

#include <windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <CL/opencl.hpp>
#include <CL/cl_gl.h>

#endif

#include <fstream>

#include "IFrameRender.h"

class GPUParallelRenderer : public IFrameRender {

    RenderingContext * context;

    cl::Device default_device;
    cl::CommandQueue queue;
    cl::Context deviceContext;
    cl::Kernel kernel;

    cl::NDRange globalRange;
    
    size_t dataSize;
    cl::Buffer pixelBuffer;
    
    int objectBufferSize;
    cl::Buffer objectBuffer;
    cl::Buffer objectsCountBuffer;

    cl::Buffer cameraBuffer;
    cl::Buffer sunBuffer;

    cl::Device GetDefaultCLDevice();  
    cl::Program FetchProgram();

public:

    void Init(RenderingContext * _context);
    
    void Render(Color * _pixels);

    ~GPUParallelRenderer();

};
#endif