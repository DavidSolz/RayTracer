#include <iostream>
#include <cstdlib>
#include <cstring>

#include <GL/glew.h>
#include <GLFW/glfw3.h>


#define CL_HPP_TARGET_OPENCL_VERSION 200

#ifdef __APPLE__

#include <OpenGL/gl3.h>
#include <OpenCL/opencl.h>
#include <OpenGL/OpenGL.h>
#include "../OpenCL/include/CL/cl.hpp"
#include <OpenCL/cl_gl.h>
#include <OpenCL/cl_gl_ext.h>

#elif _WIN32

#include <windows.h>
#include <GL/gl.h>
#include <GL/wglew.h>
#include <CL/opencl.hpp>
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <CL/cl_gl_ext.h>

#endif

int Tmain(){
    const int width = 500;
    const int height = 500;

    const int texture_width = 64;
    const int texture_height = 64;

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

    cl::Platform platform;
    cl::Device device;

    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    if (platforms.empty()) {
        std::cerr << "No OpenCL platforms found" << std::endl;
        return EXIT_FAILURE;
    }

    platform = platforms[0];

    std::vector<cl::Device> devices;
    platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

    if (devices.empty()) {
        std::cerr << "No OpenCL devices found" << std::endl;
        return EXIT_FAILURE;
    }


#ifdef __APPLE__

    device = devices[2];

#else

    device = devices[0];

#endif

    std::string platformName;
    platform.getInfo(CL_PLATFORM_NAME, &platformName);
    fprintf(stdout, "Default Platform:\n\tName: %s\n", platformName.c_str());

    std::string deviceName;
    device.getInfo(CL_DEVICE_NAME, &deviceName);
    fprintf(stdout, "Default Device:\n\tName: %s\n", deviceName.c_str());

    std::string platformExtensions;
    platform.getInfo(CL_PLATFORM_EXTENSIONS, &platformExtensions);

#ifdef __APPLE__
    size_t found = platformExtensions.find("cl_APPLE_gl_sharing");
    if (found == std::string::npos) {
        fprintf(stderr, "Platform does not support cl_APPLE_gl_sharing\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    fprintf(stdout, "Platform supports cl_APPLE_gl_sharing\n");

#else

    size_t found = platformExtensions.find("cl_khr_gl_sharing");
    if (found == std::string::npos) {
        fprintf(stderr, "Platform does not support cl_khr_gl_sharing\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    fprintf(stdout, "Platform supports cl_khr_gl_sharing\n");

#endif

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
        CL_CONTEXT_PLATFORM, (cl_context_properties) platform,
        0
    };

#endif

    cl::Context context = cl::Context(device, properties);

    cl::CommandQueue commandQueue = cl::CommandQueue(context, device);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_width, texture_height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    cl_int error;
    cl_mem shared_mem = clCreateFromGLTexture2D(context(), CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, texture, &error);

    if(error != CL_SUCCESS){
        fprintf(stderr, "Sharing failed!\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }


	fprintf(stdout, "Successfully shared!\n");


    const char * kernel_code = R"(

        kernel void drawImage(__write_only image2d_t output, float dimm) {
            int x = get_global_id(0);
            int y = get_global_id(1);

            int width = get_global_size(0);
            int height = get_global_size(1);

            float galacticRed = cos(x * M_PI / 180.0f * dimm) * 0.5f + 0.5f;
            float galacticGreen = sin(y * M_PI / 180.0f * dimm) * 0.5f + 0.5f;
            float galacticBlue = (cos(dimm * M_PI / 180.0f) + 1.0f) * 0.5f;

            float spatialPattern = cos((x + y) * M_PI / 180.0f * dimm) * 0.5f + 0.5f;

            galacticRed *= spatialPattern;
            galacticGreen *= spatialPattern;
            galacticBlue *= spatialPattern;

            write_imagef(output, (int2)(x, y), (float4)(galacticRed, galacticGreen, galacticBlue, 1.0f));
        }



    )";

    size_t sourceSize = strlen(kernel_code);

    cl::Program::Sources sources;

    sources.push_back({kernel_code, sourceSize});

    cl::Program program = cl::Program(context, sources);

    if(program.build() != CL_SUCCESS){
        fprintf(stderr, "Build failed!\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    fprintf(stdout, "Successfully builded!\n");

    cl_float dimmer = 1e-6f;

    cl::Kernel kernel = cl::Kernel(program, "drawImage");
    kernel.setArg( 0, sizeof(cl_mem), &shared_mem);
    kernel.setArg( 1, sizeof(cl_float), &dimmer);


    while( !glfwWindowShouldClose(window)){

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        clEnqueueAcquireGLObjects(commandQueue(), 1, &shared_mem, 0, NULL, NULL);
        commandQueue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(texture_width, texture_height), cl::NDRange(16, 16));
        clEnqueueReleaseGLObjects(commandQueue(), 1, &shared_mem, 0, NULL, NULL);

        commandQueue.finish();

        dimmer += 1.0f;
        kernel.setArg( 1, sizeof(cl_float), &dimmer);

        glEnable(GL_TEXTURE_2D);

        glBegin(GL_QUADS);
        glTexCoord2i(0, 0); glVertex2f(0, 0);
        glTexCoord2i(1, 0); glVertex2f(width, 0);
        glTexCoord2i(1, 1); glVertex2f(width, height);
        glTexCoord2i(0, 1); glVertex2f(0, height);
        glEnd();

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

    glDeleteTextures(1, &texture);

    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
