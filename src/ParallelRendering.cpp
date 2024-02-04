#include "ParallelRendering.h"

void ParallelRendering::BindContext(RenderingContext * _context){

    this->context = _context;

    context->loggingService.Write(MessageType::INFO, "Configuring accelerator...");

    GetDefaultCLDevice();

#ifdef __APPLE__

    CGLContextObj glContext = CGLGetCurrentContext();
    CGLShareGroupObj shareGroup = CGLGetShareGroup(glContext);

    cl_context_properties properties[] = {
        CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE,
        (cl_context_properties)shareGroup,
        0
    };

#elif __WIN32__

    cl_context_properties properties[] = {
        CL_CONTEXT_PLATFORM, (cl_context_properties)defaultPlatform(),
        CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
        CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
        0
    };

#else

     cl_context_properties properties[] = {
        CL_GL_CONTEXT_KHR, (cl_context_properties) glXGetCurrentContext(),
        CL_GLX_DISPLAY_KHR, (cl_context_properties) glXGetCurrentDisplay(),
        CL_CONTEXT_PLATFORM, (cl_context_properties) defaultPlatform(),
        0
    };

#endif

    deviceContext = cl::Context(defaultDevice);

    queue = cl::CommandQueue(deviceContext, defaultDevice);

    context->loggingService.Write(MessageType::INFO, "Building programs...");

    cl::Program program = FetchProgram();

    if(!program()){

        context->loggingService.Write(MessageType::ISSUE, "Unable to compile kernel!");
        context->loggingService.Write(MessageType::INFO, "Accelerator configuration done");      

        return ;
    }

    dataSize = sizeof(Color) * context->width * context->height;
    pixelBuffer = cl::Buffer(deviceContext, CL_MEM_READ_WRITE, dataSize);

    context->frameCounter = 0;

    objectBufferSize = sizeof(Object)*context->objects.size();
    objectBuffer = cl::Buffer(deviceContext, CL_MEM_READ_ONLY, objectBufferSize);

    materialBufferSize = sizeof(Material) * context->materials.size();
    materialBuffer = cl::Buffer(deviceContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, materialBufferSize, context->materials.data(), NULL);

    objectsCountBuffer = cl::Buffer(deviceContext, CL_MEM_READ_ONLY, sizeof(int));

    verticesBufferSize = sizeof(Vector3) * context->mesh.vertices.size();
    verticesBuffer = cl::Buffer(deviceContext, CL_MEM_READ_ONLY, verticesBufferSize);

    kernel = cl::Kernel(program, "RayTrace");
    kernel.setArg(0, pixelBuffer);
    kernel.setArg(1, objectBuffer);
    kernel.setArg(2, materialBuffer);
    kernel.setArg(3, verticesBuffer);
    kernel.setArg(4, objectsCountBuffer);
    kernel.setArg(5, sizeof(Camera), &context->camera);
    kernel.setArg(6, sizeof(int), &context->frameCounter);


    antialiasingKernel = cl::Kernel(program, "AntiAlias");
    antialiasingKernel.setArg(0, pixelBuffer);

    globalRange = cl::NDRange(context->width, context->height);

    context->loggingService.Write(MessageType::INFO, "Determining local size...");

    DetermineLocalSize(context->width, context->height);

    context->loggingService.Write(MessageType::INFO, "Accelerator configuration done");
}

void ParallelRendering::DetermineLocalSize(const uint32_t & width, const uint32_t & height){

    std::vector<size_t> maxWorkItemSizes = defaultDevice.getInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>();
    size_t maxGroupSize = defaultDevice.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();

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
    queue.enqueueWriteBuffer(materialBuffer, CL_FALSE, 0, materialBufferSize, context->materials.data());
    queue.enqueueWriteBuffer(verticesBuffer, CL_FALSE, 0, verticesBufferSize, context->mesh.vertices.data());
    queue.enqueueWriteBuffer(objectsCountBuffer, CL_FALSE, 0, sizeof(int), &objCount);
    kernel.setArg(5, sizeof(Camera), &context->camera);
    kernel.setArg(6, sizeof(int), &context->frameCounter);

    queue.enqueueNDRangeKernel(kernel, cl::NullRange, globalRange, localRange);
    queue.enqueueNDRangeKernel(antialiasingKernel, cl::NullRange, globalRange, localRange);

    queue.finish();

    queue.enqueueReadBuffer(pixelBuffer, CL_TRUE, 0, dataSize, _pixels);

}

void ParallelRendering::GetDefaultCLDevice(){
    std::vector<cl::Platform> all_platforms;

    cl::Platform::get(&all_platforms);

    if(all_platforms.size()==0){

        context->loggingService.Write(MessageType::ISSUE, "No available platforms");
        context->loggingService.Write(MessageType::INFO, "Accelerator configuration done");

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

    defaultPlatform = all_platforms[selectedPlatform];

    std::vector<cl::Device> all_devices;

    defaultPlatform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);

    std::string platformExtensions;
    defaultPlatform.getInfo(CL_PLATFORM_EXTENSIONS, &platformExtensions);

#ifdef __APPLE__
    const char * extensionName = "cl_APPLE_gl_sharing";
#else
    const char * extensionName = "cl_khr_gl_sharing";
#endif

    size_t found = platformExtensions.find(extensionName);

    if (found == std::string::npos) {
        fprintf(stdout, "Platform does not support %s\n", extensionName);
    }else{
        fprintf(stdout, "Platform supports %s\n", extensionName);
    }

    if(all_devices.size()==0){

        context->loggingService.Write(MessageType::ISSUE, "No available devices");

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

    defaultDevice = all_devices[selectedDevice];


    char buffer[250] = {0};

    sprintf(buffer, "Discovered platform : %s", defaultPlatform.getInfo<CL_PLATFORM_NAME>().c_str());

    context->loggingService.Write(MessageType::INFO, buffer);   

    sprintf(buffer, "Selected device : %s", defaultDevice.getInfo<CL_DEVICE_NAME>().c_str());

    context->loggingService.Write(MessageType::INFO, buffer);

    sprintf(buffer, "Using : %s", defaultDevice.getInfo<CL_DEVICE_OPENCL_C_VERSION>().c_str());
    
    context->loggingService.Write(MessageType::INFO, buffer);
          
    sprintf(buffer, "Discovered max work group size : %ld", defaultDevice.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>());
    
    context->loggingService.Write(MessageType::INFO, buffer);
      
}

cl::Program ParallelRendering::FetchProgram(){

    cl::Program::Sources sources;

    std::string kernel_code;

    std::fstream input("resources/tracingkernel.cl");

    if(!input){

        
        context->loggingService.Write(MessageType::ISSUE, "Kernel can't be loaded");

        exit(-1);
    }

    std::getline(input, kernel_code,'\0');

    input.close();

    sources.push_back({kernel_code.c_str(), kernel_code.length()});

    cl::Program program(deviceContext, sources);

    char buffer[250] = {0};

    if(program.build() != CL_SUCCESS){
        std::string buildLog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(defaultDevice);

        sprintf(buffer, "Error during building program. Build log:\n%s\n", buildLog.c_str());

        context->loggingService.Write(MessageType::ISSUE, buffer);
        context->loggingService.Write(MessageType::INFO, "Accelerator configuration done");
        exit(-1);
    }

    sprintf(buffer, "Discovered programs : %s", program.getInfo<CL_PROGRAM_KERNEL_NAMES>().c_str());
    context->loggingService.Write(MessageType::INFO, buffer);

    return program;

}
