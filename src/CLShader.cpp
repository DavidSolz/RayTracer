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

    tempSize = sizeof(float) * context->width * context->height;
    LocalBuffer * depthBuffer = ComputeEnvironment::CreateBuffer(deviceContext, tempSize, CL_MEM_READ_WRITE);
    buffers.emplace_back(depthBuffer);

    tempSize = sizeof(Vector3) * context->width * context->height;
    LocalBuffer * normalBuffer = ComputeEnvironment::CreateBuffer(deviceContext, tempSize, CL_MEM_READ_WRITE);
    buffers.emplace_back(normalBuffer);

    globalRange = cl::NDRange(context->width, context->height, 1);

    cl_device_type type;
    clGetDeviceInfo(device(), CL_DEVICE_TYPE, sizeof(cl_device_type), &type, NULL);

    cl_uint preferredWorkGroupSizeMultiple;
    clGetDeviceInfo(device(), CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE , sizeof(cl_uint), &preferredWorkGroupSizeMultiple, NULL);

    context->loggingService.Write(MessageType::INFO, "Preferred warp size : %d", preferredWorkGroupSizeMultiple);

    if( type == CL_DEVICE_TYPE_CPU){
        localRange = cl::NDRange(1, 1, 1);
    }else{
        localRange = cl::NDRange(8, 4, 1);
    }

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
    intersectionKernel.setArg(3, normalBuffer->buffer);

    rayGenerationKernel.setArg(0, resources->buffer);
    rayGenerationKernel.setArg(1, rayBuffer->buffer);
    rayGenerationKernel.setArg(2, lightBuffer->buffer);
    rayGenerationKernel.setArg(3, accumulatorBuffer->buffer);
    rayGenerationKernel.setArg(4, depthBuffer->buffer);
    rayGenerationKernel.setArg(5, normalBuffer->buffer);
    rayGenerationKernel.setArg(6, sizeof(Camera), &context->camera);
    rayGenerationKernel.setArg(7, sizeof(uint32_t), &context->frameCounter);

    raytracingKernel.setArg(0, resources->buffer);
    raytracingKernel.setArg(1, rayBuffer->buffer);
    raytracingKernel.setArg(2, sampleBuffer->buffer);
    raytracingKernel.setArg(3, lightBuffer->buffer);
    raytracingKernel.setArg(4, accumulatorBuffer->buffer);
    raytracingKernel.setArg(5, colorsBuffer->buffer);
    raytracingKernel.setArg(6, normalBuffer->buffer);
    raytracingKernel.setArg(7, sizeof(Camera), &context->camera);
    raytracingKernel.setArg(8, sizeof(uint32_t), &context->frameCounter);

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
    correctionKernel.setArg(6, depthBuffer->buffer);
    correctionKernel.setArg(7, normalBuffer->buffer);

    depthKernel = ComputeEnvironment::CreateKernel(deviceContext, device, "resources/kernels/DepthMapping.cl", "DepthMapping");
    depthKernel.setArg(0, resources->buffer);
    depthKernel.setArg(1, rayBuffer->buffer);
    depthKernel.setArg(2, sampleBuffer->buffer);
    depthKernel.setArg(3, depthBuffer->buffer);
}

void CLShader::Render(Color * _pixels){

    rayGenerationKernel.setArg(6, sizeof(Camera), &context->camera);
    rayGenerationKernel.setArg(7, sizeof(uint32_t), &context->frameCounter);

    raytracingKernel.setArg(7, sizeof(Camera), &context->camera);
    raytracingKernel.setArg(8, sizeof(uint32_t), &context->frameCounter);

    correctionKernel.setArg(4, sizeof(uint32_t), &context->frameCounter);

    queue.enqueueNDRangeKernel(rayGenerationKernel, cl::NullRange, globalRange, localRange);

    queue.enqueueNDRangeKernel(intersectionKernel, cl::NullRange, globalRange, localRange);
    queue.enqueueNDRangeKernel(depthKernel, cl::NullRange, globalRange, localRange);
    queue.enqueueNDRangeKernel(raytracingKernel, cl::NullRange, globalRange, localRange);

    for(int i = 1; i < 4; ++i){
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
