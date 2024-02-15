#ifndef PARALLELRENDERING_H
#define PARALLELRENDERING_H

#define CL_HPP_TARGET_OPENCL_VERSION 200

#ifdef __APPLE__

#include <GL/glew.h>

#include <OpenCL/opencl.h>
#include <OpenGL/OpenGL.h>
#include <OpenCL/cl_gl.h>
#include "../OpenCL/include/CL/cl.hpp"

#elif __WIN32__

#include <windows.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/wglew.h>
#include <CL/opencl.hpp>
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <CL/cl_gl_ext.h>

#endif

#include <fstream>

#include "IFrameRender.h"

class ParallelRendering : public IFrameRender {

    RenderingContext * context;

    cl::Device defaultDevice;
    cl::Platform defaultPlatform;

    cl::CommandQueue queue;
    cl::Context deviceContext;
    
    cl::Kernel raytracingKernel;
    cl::Kernel transferKernel;
    cl::Kernel antialiasingKernel;

    cl::NDRange globalRange;
    cl::NDRange localRange;


    size_t dataSize;
    size_t objectBufferSize;
    size_t materialBufferSize;
    size_t verticesBufferSize;
    size_t scratchBufferSize;

    const cl_image_format format = {CL_RGBA, CL_FLOAT};

    cl_mem textureBuffer;
    cl::Buffer resourcesBuffer;
    cl::Buffer objectBuffer;
    cl::Buffer materialBuffer;
    cl::Buffer verticesBuffer;
    cl::Buffer scratchBuffer;

    void GetDefaultDevice();

    void CreateDeviceContext();

    cl::Program FetchProgram();

    void DetermineLocalSize(const uint32_t & width, const uint32_t & height);

public:

    ParallelRendering(RenderingContext * _context);

    void Render(Color * _pixels);

    ~ParallelRendering();

};
#endif
