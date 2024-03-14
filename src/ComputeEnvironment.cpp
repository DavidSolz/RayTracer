#include "ComputeEnvironment.h"

RenderingContext * ComputeEnvironment::context = NULL;

LocalBuffer * ComputeEnvironment::CreateBuffer(const cl::Context & deviceContext, const size_t & _size, const cl_mem_flags & flag){
    cl_int status;
    LocalBuffer * localBuffer = new LocalBuffer{};

    context->loggingService.Write(MessageType::INFO, "Allocating new memory buffer");

    if( _size > 0){
        localBuffer->size = _size;
        localBuffer->buffer = cl::Buffer(deviceContext, flag, _size, nullptr, &status);
    }else{
        localBuffer->size = 1;
        localBuffer->buffer = cl::Buffer(deviceContext, CL_MEM_READ_WRITE, 1, nullptr, &status);
    }

    if(status != 0){
        context->loggingService.Write(MessageType::ISSUE,"Error during buffer allocation");
        delete localBuffer;
        return NULL;
    }

    return localBuffer;
}

LocalBuffer * ComputeEnvironment::CreateBuffer(const cl::Context & deviceContext, const size_t & _size, const void * data){
    cl_int status;
    LocalBuffer * localBuffer = new LocalBuffer{};

    context->loggingService.Write(MessageType::INFO, "Allocating new read-only buffer");

    if( _size > 0){
        localBuffer->size = _size;
        localBuffer->buffer = cl::Buffer(deviceContext, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, _size, (void*)data, &status);
    }else{
        localBuffer->size = 1;
        localBuffer->buffer = cl::Buffer(deviceContext, CL_MEM_READ_WRITE, 1, nullptr, &status);
    }

    if(status != 0){
        context->loggingService.Write(MessageType::ISSUE,"Error during buffer allocation");
        delete localBuffer;
        return NULL;
    }

    return localBuffer;
}

LocalBuffer * ComputeEnvironment::CreateBuffer(const cl::Context & deviceContext, const size_t & _size, const cl_mem_flags & flag, const void * data){
    cl_int status;
    LocalBuffer * localBuffer = new LocalBuffer{};

    context->loggingService.Write(MessageType::INFO, "Allocating new data filled buffer with flag");

    if( _size > 0){
        localBuffer->size = _size;
        localBuffer->buffer = cl::Buffer(deviceContext, flag, _size, (void*)data, &status);
    }else{
        localBuffer->size = 1;
        localBuffer->buffer = cl::Buffer(deviceContext, CL_MEM_READ_WRITE, 1, nullptr, &status);
    }

    if(status != 0){
        context->loggingService.Write(MessageType::ISSUE,"Error during buffer allocation");
        delete localBuffer;
        return NULL;
    }

    return localBuffer;
}

void ComputeEnvironment::SetContext(RenderingContext * _context){
    context = _context;
}

cl::Context ComputeEnvironment::CreateDeviceContext(const cl::Device & device, const cl::Platform & platform){


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
        CL_CONTEXT_PLATFORM, (cl_context_properties)platform(),
        CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
        CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
        0
    };

#else

     cl_context_properties properties[] = {
        CL_GL_CONTEXT_KHR, (cl_context_properties) glXGetCurrentContext(),
        CL_GLX_DISPLAY_KHR, (cl_context_properties) glXGetCurrentDisplay(),
        CL_CONTEXT_PLATFORM, (cl_context_properties) platform(),
        0
    };

#endif

    cl::Context deviceContext;

    if( context->memorySharing ){
        context->loggingService.Write(MessageType::INFO, "Enabling OpenCL-OpenGL interoperability");
        deviceContext = cl::Context(device, properties);
    }else{
        deviceContext = cl::Context(device);
    }

    return deviceContext;
}

 cl::Platform ComputeEnvironment::GetDefaultPlatform(){
    std::vector<cl::Platform> all_platforms;

    cl::Platform::get(&all_platforms);

    if(all_platforms.size()==0){
        context->loggingService.Write(MessageType::ISSUE, "No available platforms");
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

    return all_platforms[selectedPlatform];
 }

cl::Device ComputeEnvironment::GetDefaultDevice(const cl::Platform & platform){

    std::vector<cl::Device> all_devices;

    platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);

    std::string platformExtensions;
    platform.getInfo(CL_PLATFORM_EXTENSIONS, &platformExtensions);

#ifdef __APPLE__
    const char * extensionName = "cl_APPLE_gl_sharing";
#else
    const char * extensionName = "cl_khr_gl_sharing";
#endif

    size_t found = platformExtensions.find(extensionName);

    if (found != std::string::npos) {
        fprintf(stdout, "Platform supports %s\n", extensionName);
    }else{
        fprintf(stdout, "Platform does not support %s\nDisabling memory sharing\n", extensionName);
        context->memorySharing = false;
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

    cl::Device defaultDevice = all_devices[selectedDevice];

    context->loggingService.Write(MessageType::INFO, "Selected platform : %s", platform.getInfo<CL_PLATFORM_NAME>().c_str());
    context->loggingService.Write(MessageType::INFO, "Selected device : %s", defaultDevice.getInfo<CL_DEVICE_NAME>().c_str());
    context->loggingService.Write(MessageType::INFO, "Version : %s", defaultDevice.getInfo<CL_DEVICE_OPENCL_C_VERSION>().c_str());
    context->loggingService.Write(MessageType::INFO, "Max local size : %.2f KB", defaultDevice.getInfo<CL_DEVICE_LOCAL_MEM_SIZE>()/1024.0f);
    context->loggingService.Write(MessageType::INFO, "Max work group size : %ld", defaultDevice.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>());

    return defaultDevice;
}

cl::Kernel ComputeEnvironment::CreateKernel(const cl::Context & deviceContext, const cl::Device & device, const char * filepath, const char * kernelName){

    cl::Program::Sources sources;

    std::string kernel_code;

    std::fstream input(filepath, std::ios::in);

    if( !input.is_open() ){
        context->loggingService.Write(MessageType::ISSUE, "File %s can't be opened", filepath);
        exit(-1);
    }

    std::getline(input, kernel_code,'\0');

    input.close();

    const char * namePointer = std::strstr(kernel_code.c_str(), kernelName);

    if( namePointer == NULL ){
        context->loggingService.Write(MessageType::ISSUE, "Program file does not contain %s kernel", kernelName);
        exit(-1);
    }

    sources.push_back({kernel_code.c_str(), kernel_code.length()});

    cl::Program program(deviceContext, sources);

    if(program.build() != CL_SUCCESS){
        std::string buildLog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
        context->loggingService.Write(MessageType::ISSUE, "Program build log : %s", buildLog.c_str());
        exit(-1);
    }
        
    context->loggingService.Write(MessageType::INFO, "Discovered programs : %s", program.getInfo<CL_PROGRAM_KERNEL_NAMES>().c_str());

    if(!program()){
        context->loggingService.Write(MessageType::ISSUE, "Unable to compile kernel!");
        exit(-1);
    }

    return cl::Kernel(program, kernelName);

}
