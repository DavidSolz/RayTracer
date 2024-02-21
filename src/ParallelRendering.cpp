#include "ParallelRendering.h"


ParallelRendering::LocalBuffer * ParallelRendering::CreateBuffer(const size_t & _size, const cl_mem_flags & flag){
    cl_int status;
    LocalBuffer * localBuffer = new LocalBuffer{};

    context->loggingService.Write(MessageType::INFO, "Allocating new memory buffer...");

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

    buffers.emplace_back(localBuffer);
    return localBuffer;
}

ParallelRendering::LocalBuffer * ParallelRendering::CreateBuffer(const size_t & _size, const void * data){
    cl_int status;
    LocalBuffer * localBuffer = new LocalBuffer{};

    context->loggingService.Write(MessageType::INFO, "Allocating new read-only buffer...");

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

    buffers.emplace_back(localBuffer);
    return localBuffer;
}

ParallelRendering::ParallelRendering(RenderingContext * _context){

    this->context = _context;

    context->loggingService.Write(MessageType::INFO, "Configuring accelerator...");

    GetDefaultDevice();

    CreateDeviceContext();

    context->loggingService.Write(MessageType::INFO, "Building programs...");

    if( context->memorySharing ){
        textureBuffer = clCreateFromGLTexture2D(deviceContext(), CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, context->textureID, NULL);
        clEnqueueAcquireGLObjects(queue(), 1, &textureBuffer, 0, NULL, NULL);
    }else{
        textureBuffer = clCreateImage2D(deviceContext(), CL_MEM_WRITE_ONLY, &format, context->width, context->height, 0, NULL, NULL);
    }

    context->frameCounter = 0;

    size_t tempSize = sizeof(Object)*context->objects.size();
    LocalBuffer * objects = CreateBuffer(tempSize, context->objects.data());

    tempSize = sizeof(Material) * context->materials.size();
    LocalBuffer * materials = CreateBuffer(tempSize, context->materials.data());

    tempSize = sizeof(Vector3) * context->mesh.vertices.size();
    LocalBuffer * vertices = CreateBuffer(tempSize, context->mesh.vertices.data());

    LocalBuffer * resources = CreateBuffer(64, CL_MEM_READ_WRITE);

    tempSize = sizeof(Color) * context->width * context->height;
    LocalBuffer * scratch = CreateBuffer(tempSize, CL_MEM_READ_WRITE);

    tempSize = sizeof(Texture) * context->textureInfo.size();
    LocalBuffer * textureInfo = CreateBuffer(tempSize, context->textureInfo.data());

    tempSize = sizeof(int) * context->textureData.size();
    LocalBuffer * textureData = CreateBuffer(tempSize, context->textureData.data());

    tempSize = sizeof(int) * context->normalMap.size();
    LocalBuffer * normalData = CreateBuffer(tempSize, context->normalMap.data());


    globalRange = cl::NDRange(context->width, context->height);

    int numObjects = context->objects.size();
    int numMaterials = context->materials.size();

    transferKernel = CreateKernel("resources/kernels/transferkernel.cl", "Transfer");
    transferKernel.setArg(0, resources->buffer);
    transferKernel.setArg(1, objects->buffer);
    transferKernel.setArg(2, materials->buffer);
    transferKernel.setArg(3, vertices->buffer);
    transferKernel.setArg(4, textureInfo->buffer);
    transferKernel.setArg(5, textureData->buffer);
    transferKernel.setArg(6, normalData->buffer);
    transferKernel.setArg(7, numObjects);
    transferKernel.setArg(8, numMaterials);

    raytracingKernel = CreateKernel("resources/kernels/tracingkernel.cl", "RayTrace");
    raytracingKernel.setArg(0, sizeof(cl_mem), &textureBuffer);
    raytracingKernel.setArg(1, resources->buffer);
    raytracingKernel.setArg(2, sizeof(Camera), &context->camera);
    raytracingKernel.setArg(3, sizeof(int), &context->frameCounter);
    raytracingKernel.setArg(4, scratch->buffer);

    context->loggingService.Write(MessageType::INFO, "Transfering data to accelerator...");
    queue.enqueueNDRangeKernel(transferKernel, cl::NullRange, cl::NDRange(1), cl::NDRange(1));
    queue.enqueueNDRangeKernel(raytracingKernel, cl::NullRange, globalRange);
    queue.finish();

    antialiasingKernel = CreateKernel("resources/kernels/antialiasingkernel.cl", "AntiAlias");
    antialiasingKernel.setArg(0, sizeof(cl_mem), &textureBuffer);
    antialiasingKernel.setArg(1, scratch->buffer);

    context->loggingService.Write(MessageType::INFO, "Accelerator configuration done");
}

ParallelRendering::~ParallelRendering(){

    if( context->memorySharing )
        clEnqueueReleaseGLObjects(queue(), 1, &textureBuffer, 0, NULL, NULL);

    for( size_t id = 0; id < buffers.size(); ++id){
        delete buffers[id];
    }

    clReleaseMemObject(textureBuffer);
}

void ParallelRendering::Render(Color * _pixels){

    raytracingKernel.setArg(2, sizeof(Camera), &context->camera);
    raytracingKernel.setArg(3, sizeof(int), &context->frameCounter);

    queue.enqueueNDRangeKernel(raytracingKernel, cl::NullRange, globalRange);
    //queue.enqueueNDRangeKernel(antialiasingKernel, cl::NullRange, globalRange);

    queue.finish();

    if( !context->memorySharing ){
        size_t origin[3] = {0, 0, 0};
        size_t region[3] = {context->width, context->height, 1};
        clEnqueueReadImage(queue(), textureBuffer, CL_TRUE, origin, region, 0, 0, _pixels, 0, NULL, NULL);
    }

}

void ParallelRendering::CreateDeviceContext(){


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

    if( context->memorySharing ){
        context->loggingService.Write(MessageType::INFO, "Enabling OpenCL-OpenGL interoperability...");
        deviceContext = cl::Context(defaultDevice, properties);
    }else{
        deviceContext = cl::Context(defaultDevice);
    }

    queue = cl::CommandQueue(deviceContext, defaultDevice);
}

void ParallelRendering::GetDefaultDevice(){
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

    defaultDevice = all_devices[selectedDevice];

    context->loggingService.Write(MessageType::INFO, "Discovered platform : %s", defaultPlatform.getInfo<CL_PLATFORM_NAME>().c_str());
    context->loggingService.Write(MessageType::INFO, "Selected device : %s", defaultDevice.getInfo<CL_DEVICE_NAME>().c_str());
    context->loggingService.Write(MessageType::INFO, "Using : %s", defaultDevice.getInfo<CL_DEVICE_OPENCL_C_VERSION>().c_str());
    context->loggingService.Write(MessageType::INFO, "Discovered max work group size : %ld", defaultDevice.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>());

}

cl::Kernel ParallelRendering::CreateKernel(const char * filepath, const char * kernelName){

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
        std::string buildLog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(defaultDevice);
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
