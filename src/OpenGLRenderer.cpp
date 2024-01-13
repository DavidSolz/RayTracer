#include "OpenGLRenderer.h"

static RenderingContext * context;
static Timer* timer;

OpenGLRenderer::OpenGLRenderer(RenderingContext * _context, const bool& _enableVSync){

    context = _context;

    cpuRender.Init(context);
    gpuRender.Init(context);

    memcpy(windowTitle, "ACC Mode", 9);
    SetRenderingService(&gpuRender);
    selection = ACC;

    if(!glfwInit()){
        printf("Cannot initialize GLFW \n");
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
        printf("Cannot create GLFW window\n");
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
        fprintf(stderr, "Failed to initialize GLEW\n");
        return;
    }

    fprintf(stdout, "========[ Window Config ]========\n\tResolution : %d x %d\n\tV-Sync : %s\n",
        context->width, context->height,
        _enableVSync ? "YES" : "NO");

    pixels = new Color[context->width * context->height];

    timer = Timer::GetInstance();

    lastMouseX = context->width/2.0f;
    lastMouseY = context->height/2.0f;

}

void OpenGLRenderer::KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods){


    float deltaTime = timer->GetDeltaTime();

    switch (key){
        case GLFW_KEY_W:
            context->camera.Move(context->camera.front, deltaTime);
            context->frameCounter=0;
            break;
        case GLFW_KEY_S:
            context->camera.Move(context->camera.front*(-1.0f), deltaTime);
            context->frameCounter=0;
            break;
        case GLFW_KEY_A:
            context->camera.Move(context->camera.right*(-1.0f), deltaTime);
            context->frameCounter=0;
            break;
        case GLFW_KEY_D:
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
        float offsetY = lastMouseY - currentY;

        lastMouseX = currentX;
        lastMouseY = currentY;

        context->camera.Rotate(offsetX, offsetY);

        context->frameCounter=0;
    }

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && selection != CPU){
        memcpy(windowTitle, "CPU Mode", 9);
        SetRenderingService(&cpuRender);
        selection = CPU;
    }

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && selection != ACC){
        memcpy(windowTitle, "ACC Mode", 9);
        SetRenderingService(&gpuRender);
        selection = ACC;
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

    sprintf(windowTitle+8, " | FPS : %d | FrameTime : %5.3f \0", fps, 1000.0f/fps);

    glfwSetWindowTitle(window, windowTitle);
    glfwSwapBuffers(window);
    glfwPollEvents();

    GLenum error = glGetError();
    if(error!=GL_NO_ERROR)
        printf("Error : %d\n", error);
}

OpenGLRenderer::~OpenGLRenderer(){

    if( pixels != NULL)
        delete[] pixels;

    glfwDestroyWindow(window);
    glfwTerminate();

    fprintf(stdout, "\nProgram exited succesfully.\n");
}
