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

    LocalBuffer * resources = ComputeEnvironment::CreateBuffer(deviceContext, 128, CL_MEM_READ_WRITE);
    buffers.emplace_back(resources);

    tempSize = sizeof(Color) * context->width * context->height;
    LocalBuffer * colorsBuffer = ComputeEnvironment::CreateBuffer(deviceContext, tempSize, CL_MEM_READ_WRITE);
    buffers.emplace_back(colorsBuffer);

    tempSize = sizeof(Sample) * context->width * context->height;
    LocalBuffer * sampleBuffer = ComputeEnvironment::CreateBuffer(deviceContext, tempSize, CL_MEM_READ_WRITE);
    buffers.emplace_back(sampleBuffer);

    tempSize = sizeof(Texture) * context->textureInfo.size();
    LocalBuffer * textureInfo = ComputeEnvironment::CreateBuffer(deviceContext, tempSize, context->textureInfo.data());
    buffers.emplace_back(textureInfo);

    tempSize = sizeof(int) * context->textureData.size();
    LocalBuffer * textureData = ComputeEnvironment::CreateBuffer(deviceContext, tempSize, context->textureData.data());
    buffers.emplace_back(textureData);

    tempSize = sizeof(BoundingBox) * context->boxes.size();
    LocalBuffer * boxBuffer = ComputeEnvironment::CreateBuffer(deviceContext, tempSize, context->boxes.data());
    buffers.emplace_back(boxBuffer);

    tempSize = sizeof(Ray) * context->width * context->height;
    LocalBuffer * rayBuffer = ComputeEnvironment::CreateBuffer(deviceContext, tempSize, CL_MEM_READ_WRITE);
    buffers.emplace_back(rayBuffer);

    size_t maxWorkItemSizes[3];
    clGetDeviceInfo(device(), CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(size_t) * 3, &maxWorkItemSizes, NULL);

    size_t localX, localY;
    size_t globalX, globalY;

    localX = std::min(maxWorkItemSizes[0], (size_t)context->width);
    localY = std::min(maxWorkItemSizes[1], (size_t)context->height);

    localX = std::max(localX, static_cast<size_t>(1));
    localY = std::max(localY, static_cast<size_t>(1));

    while( context->width % localX != 0)
        localX--;
    
    while( context->height % localY != 0)
        localY--;

    globalX = ceil(context->width / (float)localX) * localX;
    globalY = ceil(context->height / (float)localY) * localY;

    globalRange = cl::NDRange(globalX, globalY);
    localRange = cl::NDRange(localX, localY);

    int numObjects = context->objects.size();
    int numMaterials = context->materials.size();

    transferKernel = ComputeEnvironment::CreateKernel(deviceContext, device, "resources/kernels/Transfer.cl", "Transfer");
    rayGenerationKernel = ComputeEnvironment::CreateKernel(deviceContext, device, "resources/kernels/CastRays.cl", "CastRays");

    if( context->bvhAcceleration ){
        context->loggingService.Write(MessageType::INFO, "Enabling BVH accelerated kernels");
        raytracingKernel = ComputeEnvironment::CreateKernel(deviceContext, device, "resources/kernels/BVHRayTracing.cl", "ComputeLight");
    }else{
        context->loggingService.Write(MessageType::INFO, "Enabling standard accelerated kernels");
        raytracingKernel = ComputeEnvironment::CreateKernel(deviceContext, device, "resources/kernels/LinearRayTracing.cl", "ComputeLight");
    }
    
    transferKernel.setArg(0, resources->buffer);
    transferKernel.setArg(1, objects->buffer);
    transferKernel.setArg(2, materials->buffer);
    transferKernel.setArg(3, textureInfo->buffer);
    transferKernel.setArg(4, textureData->buffer);
    transferKernel.setArg(5, boxBuffer->buffer);
    transferKernel.setArg(6, rayBuffer->buffer);
    transferKernel.setArg(7, sampleBuffer->buffer);
    transferKernel.setArg(8, colorsBuffer->buffer);
    transferKernel.setArg(9, numObjects);
    transferKernel.setArg(10, numMaterials);
    transferKernel.setArg(11, sizeof(uint32_t), &context->width);
    transferKernel.setArg(12, sizeof(uint32_t), &context->height);

    rayGenerationKernel.setArg(0, resources->buffer);
    rayGenerationKernel.setArg(1, sizeof(Camera), &context->camera);
    rayGenerationKernel.setArg(2, sizeof(int), &context->frameCounter);

    raytracingKernel.setArg(0, resources->buffer);
    raytracingKernel.setArg(1, sizeof(Camera), &context->camera);
    raytracingKernel.setArg(2, sizeof(int), &context->frameCounter);

    context->loggingService.Write(MessageType::INFO, "Transfering data to accelerator");
    queue.enqueueNDRangeKernel(transferKernel, cl::NullRange, cl::NDRange(1), cl::NDRange(1));    
    queue.finish();

    filterKernel = ComputeEnvironment::CreateKernel(deviceContext, device, "resources/kernels/Filter.cl", "Filter");
    filterKernel.setArg(0, sizeof(cl_mem), &textureBuffer);
    filterKernel.setArg(1, resources->buffer);

    intersectionKernel = ComputeEnvironment::CreateKernel(deviceContext, device, "resources/kernels/Traverse.cl", "Traverse");
    intersectionKernel.setArg(0, resources->buffer);
}

void CLShader::Render(Color * _pixels){

    rayGenerationKernel.setArg(1, sizeof(Camera), &context->camera);
    rayGenerationKernel.setArg(2, sizeof(int), &context->frameCounter);

    raytracingKernel.setArg(1, sizeof(Camera), &context->camera);
    raytracingKernel.setArg(2, sizeof(int), &context->frameCounter);
    
    queue.enqueueNDRangeKernel(rayGenerationKernel, cl::NullRange, globalRange);

    queue.enqueueNDRangeKernel(intersectionKernel, cl::NullRange, globalRange);
    queue.enqueueNDRangeKernel(raytracingKernel, cl::NullRange, globalRange);
    
    queue.enqueueNDRangeKernel(filterKernel, cl::NullRange, globalRange);

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