#ifndef PARALLELRENDERING_H
#define PARALLELRENDERING_H

#define CL_HPP_TARGET_OPENCL_VERSION 200
#define CL_HPP_ENABLE_EXCEPTIONS

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

#include "IFrameRender.h"

#include <fstream>
#include <string>
#include <vector>


class ParallelRendering : public IFrameRender {

    struct LocalBuffer{
        size_t size;
        cl::Buffer buffer;
    };

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

    const cl_image_format format = {CL_RGBA, CL_FLOAT};

    cl_mem textureBuffer;

    std::vector< LocalBuffer * > buffers;

    void GetDefaultDevice();

    void CreateDeviceContext();

    /// @brief Creates buffer of specified type and size
    /// @param size 
    /// @param flag 
    /// @return buffer
    LocalBuffer * CreateBuffer(const size_t & _size, const cl_mem_flags & flag);

    /// @brief Creates read only buffer  of specified size
    /// @param size 
    /// @param data 
    /// @return read only buffer filled with data
    LocalBuffer * CreateBuffer(const size_t & _size, const void * data);

    /// @brief Creates OpenCL kernel
    /// @param filepath 
    /// @param kernelName 
    /// @return kernel object
    cl::Kernel CreateKernel(const char * filepath, const char * kernelName);

public:

    ParallelRendering(RenderingContext * _context);

    void Render(Color * _pixels);

    ~ParallelRendering();

};
#endif
