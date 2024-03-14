#ifndef COMPUTEENVIRONMENT_H
#define COMPUTEENVIRONMENT_H

#define CL_HPP_TARGET_OPENCL_VERSION 200
#define CL_HPP_ENABLE_EXCEPTIONS

#define GL_SILENCE_DEPRECATION

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

#include "RenderingContext.h"

#include <fstream>
#include <string>
#include <vector>

struct LocalBuffer{
    size_t size;
    cl::Buffer buffer;
};

class ComputeEnvironment {
private:

    static RenderingContext * context;

public:


    /// @brief Binds current rendering context 
    static void SetContext(RenderingContext * _context);

    /// @brief Creates context within defualt device
    static cl::Context CreateDeviceContext(const cl::Device & device, const cl::Platform & platform);

    /// @brief Creates buffer of specified type and size
    /// @param size 
    /// @param flag 
    /// @return buffer
    static LocalBuffer * CreateBuffer(const cl::Context & deviceContext, const size_t & _size, const cl_mem_flags & flag);

    /// @brief Creates buffer of specified type and size
    /// @param size 
    /// @param flag 
    /// @param data 
    /// @return buffer
    static LocalBuffer * CreateBuffer(const cl::Context & deviceContext, const size_t & _size, const cl_mem_flags & flag, const void * data);

    /// @brief Creates read only buffer of specified size
    /// @param size 
    /// @param data 
    /// @return read only buffer filled with data
    static LocalBuffer * CreateBuffer(const cl::Context & deviceContext, const size_t & _size, const void * data);

    /// @brief Creates OpenCL kernel
    /// @param filepath 
    /// @param kernelName 
    /// @return kernel object
    static cl::Kernel CreateKernel(const cl::Context & deviceContext, const cl::Device & device, const char * filepath, const char * kernelName);

    /// @brief Creates handle to selected device
    static cl::Device GetDefaultDevice(const cl::Platform & platform);

    /// @brief Creates handle to selected platform
    static cl::Platform GetDefaultPlatform();
};
#endif
