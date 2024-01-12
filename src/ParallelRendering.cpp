#include "ParallelRendering.h"

void ParallelRendering::Init(RenderingContext * _context){

    this->context = _context;

    default_device = GetDefaultCLDevice();
    deviceContext = cl::Context(default_device);

    queue = cl::CommandQueue(deviceContext, default_device);

    cl::Program program = FetchProgram();

    if(!program()){
        printf("Unable to compile kernel!\n");
        return ;
    }

    dataSize = sizeof(Color) * context->width * context->height;
    pixelBuffer = cl::Buffer(deviceContext, CL_MEM_READ_WRITE | CL_MEM_HOST_READ_ONLY, dataSize);

    context->frameCounter = 0;
    frameCounter = cl::Buffer(deviceContext, CL_MEM_READ_ONLY, sizeof(int));

    objectBufferSize = sizeof(Object)*context->objects.size();
    objectBuffer = cl::Buffer(deviceContext, CL_MEM_READ_ONLY, objectBufferSize);

    materialBufferSize = sizeof(Material) * context->materials.size();
    materialBuffer = cl::Buffer(deviceContext, CL_MEM_READ_ONLY, materialBufferSize);

    objectsCountBuffer = cl::Buffer(deviceContext, CL_MEM_READ_ONLY, sizeof(int));

    cameraBuffer = cl::Buffer(deviceContext, CL_MEM_READ_ONLY, sizeof(Camera));

    kernel = cl::Kernel(program, "RayTrace");
    kernel.setArg(0, pixelBuffer);
    kernel.setArg(1, objectBuffer);
    kernel.setArg(2, objectsCountBuffer);
    kernel.setArg(3, materialBuffer);
    kernel.setArg(4, cameraBuffer);
    kernel.setArg(5, frameCounter);

    globalRange = cl::NDRange(context->width, context->height);
    localRange = cl::NDRange(16, 16);

}

void ParallelRendering::Render(Color * _pixels){

    int objCount = context->objects.size();

    queue.enqueueWriteBuffer(objectBuffer, CL_FALSE, 0, objectBufferSize, context->objects.data());
    queue.enqueueWriteBuffer(objectsCountBuffer, CL_FALSE, 0, sizeof(int), &objCount);
    queue.enqueueWriteBuffer(materialBuffer, CL_FALSE, 0, materialBufferSize, context->materials.data());
    queue.enqueueWriteBuffer(cameraBuffer, CL_FALSE, 0, sizeof(Camera), &context->camera);
    queue.enqueueWriteBuffer(frameCounter, CL_FALSE, 0, sizeof(int), &context->frameCounter);

    #ifdef __APPLE__
        queue.enqueueNDRangeKernel(kernel, cl::NullRange, globalRange);
    #else
        queue.enqueueNDRangeKernel(kernel, cl::NullRange, globalRange, localRange);
    #endif
    
    queue.finish();

    queue.enqueueReadBuffer(pixelBuffer, CL_TRUE, 0, dataSize, _pixels);

}

cl::Device ParallelRendering::GetDefaultCLDevice(){
    std::vector<cl::Platform> all_platforms;

    cl::Platform::get(&all_platforms);

    if(all_platforms.size()==0){
        fprintf(stderr, "No available platforms. \n");
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
        fprintf(stderr, "No available devices. \n");
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

    fprintf(stdout, "========[ Accelerator Config ]========\nPlatform :\n\tName : %s\nDevice :\n\tVendor : %s\n\tName : %s\n\tLocal work size : %ld\n",
    all_platforms[selectedPlatform].getInfo<CL_PLATFORM_NAME>().c_str(),
    device.getInfo<CL_DEVICE_VENDOR>().c_str(),
    device.getInfo<CL_DEVICE_NAME>().c_str(),
    device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>());

    return device;
}

cl::Program ParallelRendering::FetchProgram(){

    cl::Program::Sources sources;

    std::string kernel_code;

    std::fstream input("resources/tracingkernel.cl");

    if(!input){
        fprintf(stderr,"File can't be opened\n");
        exit(-1);
    }

    std::getline(input, kernel_code,'\0');

    input.close();

    sources.push_back({kernel_code.c_str(), kernel_code.length()});

    cl::Program program(deviceContext, sources);

    if(program.build() != CL_SUCCESS){
        std::string buildLog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device);
        fprintf(stderr, "Error during building program. Build log:\n%s\n", buildLog.c_str());
        exit(-1);
    }

    fprintf(stdout, "Program :\n\tName : %s\n", program.getInfo<CL_PROGRAM_KERNEL_NAMES>().c_str());

    return program;

}
