#include "ParallelRendering.h"

void ParallelRendering::Init(RenderingContext * _context){

    this->context = _context;

    fprintf(stdout, "========[ Accelerator Config ]========\n");

    default_device = GetDefaultCLDevice();
    deviceContext = cl::Context(default_device);

    queue = cl::CommandQueue(deviceContext, default_device);

    cl::Program program = FetchProgram();

    if(!program()){
        printf("Unable to compile kernel!\n");
        return ;
    }

    dataSize = sizeof(Color) * context->width * context->height;
    pixelBuffer = cl::Buffer(deviceContext, CL_MEM_READ_WRITE, dataSize);

    context->frameCounter = 0;
    frameCounter = cl::Buffer(deviceContext, CL_MEM_READ_ONLY, sizeof(int));

    objectBufferSize = sizeof(Sphere)*context->spheres.size();
    objectBuffer = cl::Buffer(deviceContext, CL_MEM_READ_ONLY, objectBufferSize);

    objectsCountBuffer = cl::Buffer(deviceContext, CL_MEM_READ_ONLY, sizeof(int));

    cameraBuffer = cl::Buffer(deviceContext, CL_MEM_READ_ONLY, sizeof(Camera));

    kernel = cl::Kernel(program, "RenderGraphics");
    kernel.setArg(0, pixelBuffer);
    kernel.setArg(1, objectBuffer);
    kernel.setArg(2, objectsCountBuffer);
    kernel.setArg(3, cameraBuffer);
    kernel.setArg(4, frameCounter);

    globalRange = cl::NDRange(context->width, context->height);
    localRange = cl::NDRange(16, 16);
}

bool ParallelRendering::isVisible(const Sphere &sphere){

    Vector3 ray = sphere.position - context->camera.position;

    return Vector3::DotProduct(ray,ray) > sphere.radius * sphere.radius;
}

void ParallelRendering::Render(Color * _pixels){

    std::vector<Sphere> visible;

    for(int i=0; i<context->spheres.size(); ++i){
        if(isVisible(context->spheres[i]))
            visible.emplace_back(context->spheres[i]);
    }


    int objCount = context->spheres.size();

    queue.enqueueWriteBuffer(objectBuffer, CL_FALSE, 0, objectBufferSize, visible.data());
    queue.enqueueWriteBuffer(objectsCountBuffer, CL_FALSE, 0, sizeof(int), &objCount);
    queue.enqueueWriteBuffer(cameraBuffer, CL_FALSE, 0, sizeof(Camera), &context->camera);
    queue.enqueueWriteBuffer(frameCounter, CL_FALSE, 0, sizeof(int), &context->frameCounter);

    queue.enqueueNDRangeKernel(kernel, cl::NullRange, globalRange, localRange);
    queue.finish();

    queue.enqueueReadBuffer(pixelBuffer, CL_TRUE, 0, dataSize, _pixels);

    context->frameCounter++;
    visible.clear();
}

ParallelRendering::~ParallelRendering(){
    //TODO
}

cl::Device ParallelRendering::GetDefaultCLDevice(){
    std::vector<cl::Platform> all_platforms;

    cl::Platform::get(&all_platforms);

    if(all_platforms.size()<1){
        fprintf(stderr, "No available platforms. \n");
        exit(-1);
    }

    for(auto const & platform : all_platforms){
        fprintf(stdout, "Platform : %s\n", platform.getInfo<CL_PLATFORM_NAME>().c_str());
    }

    std::vector<cl::Device> all_devices;

    all_platforms[0].getDevices(CL_DEVICE_TYPE_ALL, &all_devices);

    if(all_devices.size()<1){
        fprintf(stderr, "No available devices. \n");
        exit(-1);
    }

    auto const & device = all_devices[0];
    fprintf(stdout, "Device :\n\tVendor : %s\n\tName : %s\n\tLocal work size : %ld\n",
    device.getInfo<CL_DEVICE_VENDOR>().c_str(),
    device.getInfo<CL_DEVICE_NAME>().c_str(),
    device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>());

    return device;
}

cl::Program ParallelRendering::FetchProgram(){

    cl::Program::Sources sources;

    std::string kernel_code;

    std::fstream input("tracingkernel.cl");

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

    fprintf(stdout, "Program info : \n\tName : %s\n", program.getInfo<CL_PROGRAM_KERNEL_NAMES>().c_str());

    return program;

}
