#include "ParallelRendering.h"

void ParallelRendering::Init(RenderingContext * _context){

    this->context = _context;

    default_device = GetDefaultCLDevice();
    deviceContext = cl::Context(default_device);

    queue = cl::CommandQueue(deviceContext, default_device);

    cl::Program program = FetchProgram();

    if(!program()){

        if(context->loggingService)
            context->loggingService->Write(MessageType::ISSUE, "Unable to compile kernel!");
            
        return ;
    }

    dataSize = sizeof(Color) * context->width * context->height;
    pixelBuffer = cl::Buffer(deviceContext, CL_MEM_READ_WRITE, dataSize);

    context->frameCounter = 0;
    frameCounter = cl::Buffer(deviceContext, CL_MEM_READ_ONLY, sizeof(int));

    objectBufferSize = sizeof(Object)*context->objects.size();
    objectBuffer = cl::Buffer(deviceContext, CL_MEM_READ_ONLY, objectBufferSize);

    materialBufferSize = sizeof(Material) * context->materials.size();
    materialBuffer = cl::Buffer(deviceContext, CL_MEM_READ_ONLY, materialBufferSize);

    objectsCountBuffer = cl::Buffer(deviceContext, CL_MEM_READ_ONLY, sizeof(int));

    cameraBuffer = cl::Buffer(deviceContext, CL_MEM_READ_ONLY, sizeof(Camera));

    verticesBufferSize = sizeof(Vector3) * context->mesh.vertices.size();
    verticesBuffer = cl::Buffer(deviceContext, CL_MEM_READ_ONLY, verticesBufferSize);


    kernel = cl::Kernel(program, "RayTrace");
    kernel.setArg(0, pixelBuffer);
    kernel.setArg(1, objectBuffer);
    kernel.setArg(2, objectsCountBuffer);
    kernel.setArg(3, materialBuffer);
    kernel.setArg(4, cameraBuffer);
    kernel.setArg(5, frameCounter);
    kernel.setArg(6, verticesBuffer);

    antialiasingKernel = cl::Kernel(program, "AntiAlias");
    antialiasingKernel.setArg(0, pixelBuffer);

    globalRange = cl::NDRange(context->width, context->height);

    DetermineLocalSize(context->width, context->height);

}

void ParallelRendering::DetermineLocalSize(const uint32_t & width, const uint32_t & height){

    std::vector<size_t> maxWorkItemSizes = default_device.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
    size_t maxGroupSize = default_device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();

    uint32_t rangeX, rangeY;

    rangeX = static_cast<uint32_t>(maxWorkItemSizes[0]);
    rangeY = static_cast<uint32_t>(maxWorkItemSizes[1]);

    for (uint32_t x = rangeX; x > 0; --x) {
        if (width % x == 0) {
            rangeX = x;
            break;
        }
    }

    for (uint32_t y = rangeY; y > 0; --y) {
        if (height % y == 0 && rangeX * y <= maxGroupSize) {
            rangeY = y;
            break;
        }
    }


    // This should be redone to support 1D groups
    if(maxWorkItemSizes[1]==1)
        rangeX = 100;


    rangeX = std::max(rangeX, (uint32_t)1);
    rangeY = std::max(rangeY, (uint32_t)1);

    localRange = cl::NDRange(rangeX, rangeY);
}

void ParallelRendering::Render(Color * _pixels){

    int objCount = context->objects.size();

    queue.enqueueWriteBuffer(objectBuffer, CL_FALSE, 0, objectBufferSize, context->objects.data());
    queue.enqueueWriteBuffer(objectsCountBuffer, CL_FALSE, 0, sizeof(int), &objCount);
    queue.enqueueWriteBuffer(materialBuffer, CL_FALSE, 0, materialBufferSize, context->materials.data());
    queue.enqueueWriteBuffer(cameraBuffer, CL_FALSE, 0, sizeof(Camera), &context->camera);
    queue.enqueueWriteBuffer(frameCounter, CL_FALSE, 0, sizeof(int), &context->frameCounter);
    queue.enqueueWriteBuffer(verticesBuffer, CL_FALSE, 0, verticesBufferSize, context->mesh.vertices.data());

    queue.enqueueNDRangeKernel(kernel, cl::NullRange, globalRange, localRange);
    queue.enqueueNDRangeKernel(antialiasingKernel, cl::NullRange, globalRange, localRange);

    queue.finish();

    queue.enqueueReadBuffer(pixelBuffer, CL_TRUE, 0, dataSize, _pixels);

}

cl::Device ParallelRendering::GetDefaultCLDevice(){
    std::vector<cl::Platform> all_platforms;

    cl::Platform::get(&all_platforms);

    if(all_platforms.size()==0){

        if(context->loggingService)
            context->loggingService->Write(MessageType::ISSUE, "No available platforms");

        exit(-1);
    }

    int selectedPlatform = 0;

    fprintf(stdout, "========[ CL Setup ]========\nPlatforms : \n");
    for (size_t i = 0; i < all_platforms.size(); ++i) {
        fprintf(stdout, "\t[%i] : %s\n", i, all_platforms[i].getInfo<CL_PLATFORM_NAME>().c_str());
    }

    if(all_platforms.size()>1){
        fprintf(stdout, "Enter desired platform id : ");
        scanf("%d", &selectedPlatform);

        selectedPlatform = std::max(0, std::min(selectedPlatform, (int)all_platforms.size()-1));
    }else{
        fprintf(stdout, "> Selected default platform.\n");
    }

    std::vector<cl::Device> all_devices;

    all_platforms[selectedPlatform].getDevices(CL_DEVICE_TYPE_ALL, &all_devices);

    if(all_devices.size()==0){

        if(context->loggingService)
            context->loggingService->Write(MessageType::ISSUE, "No available devices");

        exit(-1);
    }

    int selectedDevice = 0;
    fprintf(stdout, "Devices : \n");
    for (size_t i = 0; i < all_devices.size(); ++i) {
        fprintf(stdout, "\t[%i] : %s\n", i, all_devices[i].getInfo<CL_DEVICE_NAME>().c_str());
    }

    if(all_devices.size()>1){
        fprintf(stdout, "Enter desired device id : ");
        scanf("%d", &selectedDevice);

        selectedDevice = std::max(0, std::min(selectedDevice, (int)all_devices.size()-1));
    }else{
        fprintf(stdout, "> Selected default device.\n");
    }

    auto const & device = all_devices[selectedDevice];

    char buffer[250] = {0};

    sprintf(buffer, "========[ Accelerator Config ]========\nPlatform :\n\tName : %s\nDevice :\n\tVendor : %s\n\tName : %s\n\tVersion : %s\n\tLocal work size : %ld\n",
    all_platforms[selectedPlatform].getInfo<CL_PLATFORM_NAME>().c_str(),
    device.getInfo<CL_DEVICE_VENDOR>().c_str(),
    device.getInfo<CL_DEVICE_NAME>().c_str(),
    device.getInfo<CL_DEVICE_OPENCL_C_VERSION>().c_str(),
    device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>());

    if(context->loggingService)
            context->loggingService->Write(MessageType::INFO, buffer);
          

    return device;
}

cl::Program ParallelRendering::FetchProgram(){

    cl::Program::Sources sources;

    std::string kernel_code;

    std::fstream input("resources/tracingkernel.cl");

    if(!input){

        if(context->loggingService)
            context->loggingService->Write(MessageType::ISSUE, "Kernel can't be loaded");

        exit(-1);
    }

    std::getline(input, kernel_code,'\0');

    input.close();

    sources.push_back({kernel_code.c_str(), kernel_code.length()});

    cl::Program program(deviceContext, sources);

    char buffer[250] = {0};

    if(program.build() != CL_SUCCESS){
        std::string buildLog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device);

        sprintf(buffer, "Error during building program. Build log:\n%s\n", buildLog.c_str());

        if(context->loggingService)
            context->loggingService->Write(MessageType::ISSUE, buffer);

        exit(-1);
    }

    sprintf(buffer, "Program :\n\tName : %s\n", program.getInfo<CL_PROGRAM_KERNEL_NAMES>().c_str());

    if(context->loggingService)
            context->loggingService->Write(MessageType::INFO, buffer);

    return program;

}
