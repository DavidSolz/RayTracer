#ifndef CLSHADER_H
#define CLSHADER_H

#include "ComputeShader.h"
#include "ComputeEnvironment.h"
#include "Ray.h"
#include "Sample.h"
#include <vector>

class CLShader : public ComputeShader{
private:
    
    cl::Platform platform;
    cl::Device device;
    cl::Context deviceContext;
    cl::CommandQueue queue;

    cl::Kernel rayGenerationKernel;
    cl::Kernel transferKernel;
    cl::Kernel intersectionKernel;
    cl::Kernel raytracingKernel;
    cl::Kernel filterKernel;

    std::vector< LocalBuffer* > buffers;

    const cl_image_format format = {CL_RGBA, CL_FLOAT};

    cl_mem textureBuffer;

    cl::NDRange globalRange;
    cl::NDRange localRange;

public:

    CLShader(RenderingContext * _context);

    void Render(Color * _pixels);

    ~CLShader();

};



#endif