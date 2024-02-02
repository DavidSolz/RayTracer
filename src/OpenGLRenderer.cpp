#include "OpenGLRenderer.h"

static RenderingContext * context;
static Timer* timer;

OpenGLRenderer::OpenGLRenderer(RenderingContext * _context, const bool & _enableVSync){

    context = _context;

    context->loggingService.Write(MessageType::INFO, "Configuring window...");

    cpuRender.Init(context);
    gpuRender.Init(context);

    memcpy(windowTitle, "ACC Mode", 9);
    SetRenderingService(&gpuRender);
    selection = ACC;

    if( glfwInit() == GLFW_FALSE ){

        context->loggingService.Write(MessageType::ISSUE, "Cannot initialize GLFW");
        context->loggingService.Write(MessageType::INFO, "Window configuration done");

        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(context->width, context->height, "RayTracer", NULL, NULL);

    if(!window){
        glfwTerminate();

        context->loggingService.Write(MessageType::ISSUE, "Cannot create GLFW window");
        context->loggingService.Write(MessageType::INFO, "Window configuration done");

        return;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(_enableVSync);

    glfwSetKeyCallback(window, KeyboardCallback);

    glLoadIdentity();
    glOrtho(0, context->width, 0, context->height, 0, context->depth);

    if (glewInit() != GLEW_OK) {
        glfwDestroyWindow(window);
        glfwTerminate();

        context->loggingService.Write(MessageType::INFO, "Failed to initialize GLEW");
        context->loggingService.Write(MessageType::INFO, "Window configuration done");

        return;
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, context->width, context->height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    context->loggingService.Write(MessageType::INFO, "Creating texture buffer");

    char buffer[200] = {0};


    sprintf(buffer, "Current resolution : %d x %d", context->width, context->height);

    context->loggingService.Write(MessageType::INFO, buffer);

    sprintf(buffer, "Checking for V-Sync : %s", _enableVSync?"enabled":"disabled");

    context->loggingService.Write(MessageType::INFO, buffer);

    context->loggingService.Write(MessageType::INFO, "Window configuration done");


    pixels = new Color[context->width * context->height];

    timer = Timer::GetInstance();

    lastMouseX = context->width/2.0f;
    lastMouseY = context->height/2.0f;

}

void OpenGLRenderer::KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods){


    float deltaTime = timer->GetDeltaTime();

    switch (key){
        case GLFW_KEY_UP:
            context->camera.Move(context->camera.front, deltaTime);
            context->frameCounter=0;
            break;
        case GLFW_KEY_DOWN:
            context->camera.Move(context->camera.front*(-1.0f), deltaTime);
            context->frameCounter=0;
            break;
        case GLFW_KEY_LEFT:
            context->camera.Move(context->camera.right*(-1.0f), deltaTime);
            context->frameCounter=0;
            break;
        case GLFW_KEY_RIGHT:
            context->camera.Move(context->camera.right, deltaTime);
            context->frameCounter=0;
            break;
        case GLFW_KEY_LEFT_SHIFT:
            context->camera.Move(worldUp*(-1.0f), deltaTime);
            context->frameCounter=0;
            break;
        case GLFW_KEY_SPACE:
            context->camera.Move(worldUp, deltaTime);
            context->frameCounter=0;
            break;
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, true);
            break;
        default:
            break;
    }

}

void OpenGLRenderer::ProcessInput(){

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        double currentX, currentY;

        glfwGetCursorPos(window, &currentX, &currentY);

        float offsetX = lastMouseX - currentX;
        float offsetY = currentY - lastMouseY;

        lastMouseX = currentX;
        lastMouseY = currentY;

        context->camera.Rotate(offsetX, offsetY);

        context->frameCounter=0;
    }

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && selection != CPU){
        memcpy(windowTitle, "CPU Mode", 9);
        SetRenderingService(&cpuRender);
        selection = CPU;
        context->frameCounter=0;
    }

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && selection != ACC){
        memcpy(windowTitle, "ACC Mode", 9);
        SetRenderingService(&gpuRender);
        selection = ACC;
        context->frameCounter=0;
    }

}

bool OpenGLRenderer::ShouldClose(){
    return glfwWindowShouldClose(window);
}

void OpenGLRenderer::SetRenderingService(IFrameRender * _service){
    this->renderingService = _service;
}

void OpenGLRenderer::Update(){

    ProcessInput();

    timer->TicTac();

    renderingService->Render(pixels);

    glDrawPixels(context->width, context->height, GL_RGBA, GL_FLOAT, pixels);

    context->frameCounter++;

    uint32_t fps = timer->GetFrameCount();

    sprintf(windowTitle+8, " | FPS : %d | Frametime : %5.3f ms\0", fps, 1000.0f/fps);

    glfwSetWindowTitle(window, windowTitle);
    glfwSwapBuffers(window);
    glfwPollEvents();

    GLenum error = glGetError();
    if(error!=GL_NO_ERROR){

        context->loggingService.Write(MessageType::ISSUE, "OpenGL buffer error");
        return;
        
    }

}

OpenGLRenderer::~OpenGLRenderer(){

    if( pixels != NULL)
        delete[] pixels;

    glDeleteTextures(1, &textureID);

    glfwDestroyWindow(window);
    glfwTerminate();

}
