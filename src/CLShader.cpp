#include "CLShader.h"


CLShader::CLShader(RenderingContext * _context) : ComputeShader(_context){

    platform = ComputeEnvironment::GetDefaultPlatform();
    device = ComputeEnvironment::GetDefaultDevice(platform);
    deviceContext = ComputeEnvironment::CreateDeviceContext(device, platform);
    queue = cl::CommandQueue(deviceContext, device);

    context->loggingService.Write(MessageType::INFO, "Binding buffers and kernels");

    if( context->memorySharing ){
        textureBuffer = clCreateFromGLTexture2D(deviceContext(), CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, context->textureID, NULL);
        clEnqueueAcquireGLObjects(queue(), 1, &textureBuffer, 0, NULL, NULL);
    }else{
        textureBuffer = clCreateImage2D(deviceContext(), CL_MEM_WRITE_ONLY, &format, context->width, context->height, 0, NULL, NULL);
    }

    size_t tempSize = sizeof(Object)*context->objects.size();
    LocalBuffer * objects = ComputeEnvironment::CreateBuffer(deviceContext, tempSize, context->objects.data());
    buffers.emplace_back(objects);

    tempSize = sizeof(Material) * context->materials.size();
    LocalBuffer * materials = ComputeEnvironment::CreateBuffer(deviceContext, tempSize, context->materials.data());
    buffers.emplace_back(materials);

    LocalBuffer * resources = ComputeEnvironment::CreateBuffer(deviceContext, 64, CL_MEM_READ_WRITE);
    buffers.emplace_back(resources);

    tempSize = sizeof(Color) * context->width * context->height;
    LocalBuffer * scratch = ComputeEnvironment::CreateBuffer(deviceContext, tempSize, CL_MEM_READ_WRITE);
    buffers.emplace_back(scratch);

    tempSize = sizeof(Texture) * context->textureInfo.size();
    LocalBuffer * textureInfo = ComputeEnvironment::CreateBuffer(deviceContext, tempSize, context->textureInfo.data());
    buffers.emplace_back(textureInfo);

    tempSize = sizeof(int) * context->textureData.size();
    LocalBuffer * textureData = ComputeEnvironment::CreateBuffer(deviceContext, tempSize, context->textureData.data());
    buffers.emplace_back(textureData);

    tempSize = sizeof(BoundingBox) * context->boxes.size();
    LocalBuffer * boxBuffer = ComputeEnvironment::CreateBuffer(deviceContext, tempSize, context->boxes.data());
    buffers.emplace_back(boxBuffer);

    globalRange = cl::NDRange(context->width, context->height);

    int numObjects = context->objects.size();
    int numMaterials = context->materials.size();

    transferKernel = ComputeEnvironment::CreateKernel(deviceContext, device, "resources/kernels/Transfer.cl", "Transfer");
    
    if( context->bvhAcceleration ){
        context->loggingService.Write(MessageType::INFO, "Enabling BVH accelerated kernels");
        raytracingKernel = ComputeEnvironment::CreateKernel(deviceContext, device, "resources/kernels/BVHRayTracing.cl", "ComputeLight");
        texturingKernel = ComputeEnvironment::CreateKernel(deviceContext, device, "resources/kernels/BVHTexturing.cl", "ApplyTexture");
    }else{
        context->loggingService.Write(MessageType::INFO, "Enabling standard accelerated kernels");
        raytracingKernel = ComputeEnvironment::CreateKernel(deviceContext, device, "resources/kernels/LinearRayTracing.cl", "ComputeLight");
        texturingKernel = ComputeEnvironment::CreateKernel(deviceContext, device, "resources/kernels/LinearTexturing.cl", "ApplyTexture");
    }
    
    raytracingKernel.setArg(0, resources->buffer);
    raytracingKernel.setArg(1, sizeof(Camera), &context->camera);
    raytracingKernel.setArg(2, sizeof(int), &context->frameCounter);
    raytracingKernel.setArg(3, scratch->buffer);

    texturingKernel.setArg(0, sizeof(cl_mem), &textureBuffer);
    texturingKernel.setArg(1, resources->buffer);
    texturingKernel.setArg(2, sizeof(Camera), &context->camera);
    texturingKernel.setArg(3, scratch->buffer);

    transferKernel.setArg(0, resources->buffer);
    transferKernel.setArg(1, objects->buffer);
    transferKernel.setArg(2, materials->buffer);
    transferKernel.setArg(3, textureInfo->buffer);
    transferKernel.setArg(4, textureData->buffer);
    transferKernel.setArg(5, boxBuffer->buffer);
    transferKernel.setArg(6, numObjects);
    transferKernel.setArg(7, numMaterials);

    context->loggingService.Write(MessageType::INFO, "Transfering data to accelerator");
    queue.enqueueNDRangeKernel(transferKernel, cl::NullRange, cl::NDRange(1), cl::NDRange(1));
    queue.enqueueNDRangeKernel(raytracingKernel, cl::NullRange, globalRange);
    queue.finish();

    antialiasingKernel = ComputeEnvironment::CreateKernel(deviceContext, device, "resources/kernels/AntiAlias.cl", "AntiAlias");
    antialiasingKernel.setArg(0, sizeof(cl_mem), &textureBuffer);
    antialiasingKernel.setArg(1, scratch->buffer);

}

void CLShader::Render(Color * _pixels){

    raytracingKernel.setArg(1, sizeof(Camera), &context->camera);
    raytracingKernel.setArg(2, sizeof(int), &context->frameCounter);
    
    texturingKernel.setArg(2, sizeof(Camera), &context->camera);

    queue.enqueueNDRangeKernel(raytracingKernel, cl::NullRange, globalRange);
    queue.enqueueNDRangeKernel(texturingKernel, cl::NullRange, globalRange);
    //TODO : antialiasing
    //queue.enqueueNDRangeKernel(antialiasingKernel, cl::NullRange, globalRange);

    queue.finish();

    if( !context->memorySharing ){
        size_t origin[3] = {0, 0, 0};
        size_t region[3] = {context->width, context->height, 1};
        clEnqueueReadImage(queue(), textureBuffer, CL_TRUE, origin, region, 0, 0, _pixels, 0, NULL, NULL);
    }


}

CLShader::~CLShader(){

    if( context->memorySharing )
        clEnqueueReleaseGLObjects(queue(), 1, &textureBuffer, 0, NULL, NULL);

    context->loggingService.Write(MessageType::INFO, "Deallocating buffers");

    for(int32_t id = 0; id < buffers.size(); ++id)
        delete buffers[id];

    clReleaseMemObject(textureBuffer);

}