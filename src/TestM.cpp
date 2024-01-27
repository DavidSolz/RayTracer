#include <iostream>
#include <cstring>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define CL_HPP_TARGET_OPENCL_VERSION 200

#ifdef __APPLE__

#include <GL/gl.h>
#include <GL/wglew.h>
#include <OpenCL/opencl.h>
#include <OpenCL/cl_gl.h>
#include <CL/cl_gl_ext.h>
#include "../OpenCL/include/CL/cl.hpp"

#elif __WIN32__

#include <windows.h>
#include <GL/gl.h>
#include <GL/wglew.h>
#include <CL/opencl.hpp>
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <CL/cl_gl_ext.h>

#endif

int main(){
    const int width = 640;
    const int height = 480;

    const int texture_width = 4;
    const int texture_height = 4;

    if(!glfwInit()){
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow * window = glfwCreateWindow(width, height, "Interop", NULL, NULL);

    if(!window){
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glLoadIdentity();
    glOrtho(0, width, 0, height, -1.0f, 1.0f);

    if (glewInit() != GLEW_OK) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    cl_platform_id platform;
    cl_device_id device;

    clGetPlatformIDs(1, &platform, NULL);

    clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, &device, NULL);

    size_t platformNameSize;
    clGetPlatformInfo(platform, CL_PLATFORM_NAME, 0, NULL, &platformNameSize);
    char* platformName = (char*)malloc(platformNameSize);
    clGetPlatformInfo(platform, CL_PLATFORM_NAME, platformNameSize, platformName, NULL);
    fprintf(stdout, "Default Platform:\n\tName: %s\n", platformName);
    free(platformName);

    size_t deviceNameSize;
    clGetDeviceInfo(device, CL_DEVICE_NAME, 0, NULL, &deviceNameSize);
    char* deviceName = (char*)malloc(deviceNameSize);
    clGetDeviceInfo(device, CL_DEVICE_NAME, deviceNameSize, deviceName, NULL);
    fprintf(stdout, "Default Device:\n\tName: %s\n", deviceName);
    free(deviceName);

    cl_int status;

    char extension_string[1024];
    memset(extension_string, ' ', 1024);
    
    clGetPlatformInfo( 0, CL_PLATFORM_EXTENSIONS, sizeof(extension_string), extension_string, NULL);

    char *extStringStart = NULL;
    extStringStart = strstr(extension_string, "cl_khr_gl_sharing");

    if(extStringStart != 0)
        fprintf(stderr, "Platform does support cl_khr_gl_sharing\n");

    cl_context_properties cps[] = 
    { 
        CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 
        CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
        CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
        0 
    };

    cl_context context = clCreateContext(cps, 1, &device, NULL, NULL, &status);

    if (context == NULL) {
        fprintf(stderr, "Failed to create OpenCL context\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    clGetGLContextInfoKHR_fn pclGetGLContextInfoKHR = (clGetGLContextInfoKHR_fn)clGetExtensionFunctionAddressForPlatform(0, "clGetGLContextInfoKHR");

    size_t bytes = 0;
	pclGetGLContextInfoKHR(cps, CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR, 0, NULL, &bytes);

    if( bytes == 0 ){
        fprintf(stderr, "Can't find any compatible devices\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    cl_command_queue commandQueue = clCreateCommandQueue(context, device, 0, &status);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, texture_width, texture_height);

    unsigned char pixels[4 * texture_width * texture_height] = {0};

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture_width, texture_height, GL_RGBA, GL_UNSIGNED_BYTE, (void*)pixels);     

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);


    cl_GLuint g_RGBAbufferGLBindName = texture;
    cl_mem shared_mem = clCreateFromGLTexture(context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, g_RGBAbufferGLBindName, &status);

    if(status != 0){
        fprintf(stderr, "Sharing failed!\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }


	fprintf(stdout, "Successfully shared!\n");
		

    const char * kernel_code = R"(

        kernel void drawImage(__write_only image2d_t output, float dimm){
            int x = get_global_id(0);
            int y = get_global_id(1);

            write_imagef(output, (int2)(x, y), (float4)(dimm, dimm, dimm, 1.f));
            
        }

    )";

    size_t sourceSize = strlen(kernel_code);

    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&kernel_code, &sourceSize, &status);

    status = clBuildProgram(program, 1, &device, NULL, NULL, NULL);

    if(status != 0){
        fprintf(stderr, "Build failed!\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    fprintf(stdout, "Successfully builded!\n");

    cl_kernel kernel = clCreateKernel(program, "drawImage", &status);

    float dimmer = 0.0f;
    size_t global_dim[2];

    while( !glfwWindowShouldClose(window)){

        glClear(GL_COLOR_BUFFER_BIT);

        clEnqueueAcquireGLObjects(commandQueue, 1, &shared_mem, 0, 0, 0);

        clSetKernelArg(kernel, 0, sizeof(cl_mem), &shared_mem);
        clSetKernelArg(kernel, 1, sizeof(cl_float), &dimmer);

        global_dim[0] = texture_width;
        global_dim[1] = texture_height;

        clEnqueueNDRangeKernel(commandQueue, kernel, 2, NULL, global_dim, NULL, 0, NULL, NULL);
        clEnqueueReleaseGLObjects(commandQueue, 1, &shared_mem, 0, NULL, NULL);

        clFinish(commandQueue); 

        dimmer *= (dimmer < 1.0f);
        dimmer += .001f;

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, g_RGBAbufferGLBindName);

        glBegin(GL_QUADS);
        glTexCoord2i(0, 0); glVertex2f(0, 0);
        glTexCoord2i(1, 0); glVertex2f(width, 0);
        glTexCoord2i(1, 1); glVertex2f(width, height);
        glTexCoord2i(0, 1); glVertex2f(0, height);
        glEnd();

        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);

        glfwSwapBuffers(window);
        glfwPollEvents();

        GLenum error = glGetError();

        if(error!=GL_NO_ERROR){
            fprintf(stderr, "OpenGL buffer error : %d\n", error);
            break;
        }

    }

    clReleaseMemObject(shared_mem);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(commandQueue);
	clReleaseContext(context);

    glDeleteTextures(1, &texture);

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}