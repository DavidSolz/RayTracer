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

    tempSize = sizeof(Color) * context->width * context->height;
    LocalBuffer * lightBuffer = ComputeEnvironment::CreateBuffer(deviceContext, tempSize, CL_MEM_READ_WRITE);
    buffers.emplace_back(lightBuffer);

    tempSize = sizeof(Color) * context->width * context->height;
    LocalBuffer * accumulatorBuffer = ComputeEnvironment::CreateBuffer(deviceContext, tempSize, CL_MEM_READ_WRITE);
    buffers.emplace_back(accumulatorBuffer);

    size_t maxWorkItemSizes[3];
    clGetDeviceInfo(device(), CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(size_t) * 3, &maxWorkItemSizes, NULL);

    size_t localSize;
    size_t globalSize;

    globalRange = cl::NDRange(context->width * context->height);
    localRange = cl::NDRange(16);

    int numObjects = context->objects.size();
    int numMaterials = context->materials.size();

    transferKernel = ComputeEnvironment::CreateKernel(deviceContext, device, "resources/kernels/Transfer.cl", "Transfer");
    rayGenerationKernel = ComputeEnvironment::CreateKernel(deviceContext, device, "resources/kernels/CastRays.cl", "CastRays");
    raytracingKernel = ComputeEnvironment::CreateKernel(deviceContext, device, "resources/kernels/RayTrace.cl", "RayTrace");

    if( context->bvhAcceleration ){
        context->loggingService.Write(MessageType::INFO, "Enabling BVH accelerated traversal kernel");
        intersectionKernel = ComputeEnvironment::CreateKernel(deviceContext, device, "resources/kernels/BVHTraverse.cl", "Traverse");
    }else{
        context->loggingService.Write(MessageType::INFO, "Enabling linear traversal kernel");
        intersectionKernel = ComputeEnvironment::CreateKernel(deviceContext, device, "resources/kernels/LinearTraverse.cl", "Traverse");
    }

    transferKernel.setArg(0, resources->buffer);
    transferKernel.setArg(1, objects->buffer);
    transferKernel.setArg(2, materials->buffer);
    transferKernel.setArg(3, textureInfo->buffer);
    transferKernel.setArg(4, textureData->buffer);
    transferKernel.setArg(5, boxBuffer->buffer);
    transferKernel.setArg(6, numObjects);
    transferKernel.setArg(7, numMaterials);
    transferKernel.setArg(8, sizeof(uint32_t), &context->width);
    transferKernel.setArg(9, sizeof(uint32_t), &context->height);

    intersectionKernel.setArg(0, resources->buffer);
    intersectionKernel.setArg(1, rayBuffer->buffer);
    intersectionKernel.setArg(2, sampleBuffer->buffer);

    rayGenerationKernel.setArg(0, resources->buffer);
    rayGenerationKernel.setArg(1, rayBuffer->buffer);
    rayGenerationKernel.setArg(2, lightBuffer->buffer);
    rayGenerationKernel.setArg(3, accumulatorBuffer->buffer);
    rayGenerationKernel.setArg(4, sizeof(Camera), &context->camera);
    rayGenerationKernel.setArg(5, sizeof(uint32_t), &context->frameCounter);

    raytracingKernel.setArg(0, resources->buffer);
    raytracingKernel.setArg(1, rayBuffer->buffer);
    raytracingKernel.setArg(2, sampleBuffer->buffer);
    raytracingKernel.setArg(3, lightBuffer->buffer);
    raytracingKernel.setArg(4, accumulatorBuffer->buffer);
    raytracingKernel.setArg(5, sizeof(Camera), &context->camera);
    raytracingKernel.setArg(6, sizeof(uint32_t), &context->frameCounter);

    context->loggingService.Write(MessageType::INFO, "Transfering data to accelerator");
    queue.enqueueNDRangeKernel(transferKernel, cl::NullRange, cl::NDRange(1), cl::NDRange(1));
    queue.finish();

    correctionKernel = ComputeEnvironment::CreateKernel(deviceContext, device, "resources/kernels/ImageCorrection.cl", "ImageCorrection");
    correctionKernel.setArg(0, sizeof(cl_mem), &textureBuffer);
    correctionKernel.setArg(1, resources->buffer);
    correctionKernel.setArg(2, accumulatorBuffer->buffer);
    correctionKernel.setArg(3, colorsBuffer->buffer);
    correctionKernel.setArg(4, sizeof(uint32_t), &context->frameCounter);
    correctionKernel.setArg(5, sizeof(float), &context->gamma);


}

void CLShader::Render(Color * _pixels){

    rayGenerationKernel.setArg(4, sizeof(Camera), &context->camera);
    rayGenerationKernel.setArg(5, sizeof(uint32_t), &context->frameCounter);

    raytracingKernel.setArg(5, sizeof(Camera), &context->camera);
    raytracingKernel.setArg(6, sizeof(uint32_t), &context->frameCounter);

    correctionKernel.setArg(4, sizeof(uint32_t), &context->frameCounter);

    queue.enqueueNDRangeKernel(rayGenerationKernel, cl::NullRange, globalRange, localRange);

    for(int i=0; i < 4; ++i){
        queue.enqueueNDRangeKernel(intersectionKernel, cl::NullRange, globalRange, localRange);
        queue.enqueueNDRangeKernel(raytracingKernel, cl::NullRange, globalRange, localRange);
    }

    queue.enqueueNDRangeKernel(correctionKernel, cl::NullRange, globalRange, localRange);

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
