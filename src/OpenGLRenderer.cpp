#include "OpenGLRenderer.h"

OpenGLRenderer::OpenGLRenderer(RenderingContext * _context, const bool& _enableVSync){

    this->context = _context;

    cpuRender.Init(context);
    gpuRender.Init(context);

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

}


void OpenGLRenderer::ProcessInput(){

    Vector3 direction = {0, 0, 0};

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        double currentX, currentY;

        glfwGetCursorPos(window, &currentX, &currentY);

        direction.x = currentX - context->camera.position.x;
        direction.y = context->camera.position.y - currentY;

        direction = direction.Normalize();
        
        context->frameCounter=0;
    }

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            direction = context->camera.front;
            context->frameCounter=0;
    }

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
            direction = context->camera.front * -1.0f;
            context->frameCounter=0;
    }

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && selection != CPU){
        fprintf(stdout, "Switching to cpu context.\n");
        SetRenderingService(&cpuRender);
        selection = CPU;
    }

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && selection != ACC){
        fprintf(stdout, "Switching to accelerator context.\n");
        SetRenderingService(&gpuRender);
        selection = ACC;
    }


    context->camera.position = context->camera.position + direction * context->camera.movementSpeed * timer->GetDeltaTime();

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

    glfwSwapBuffers(window);
    glfwPollEvents();

    GLenum error = glGetError();
    if(error!=GL_NO_ERROR)
        printf("Error : %d\n", error);
}

OpenGLRenderer::~OpenGLRenderer(){

    delete[] pixels;

    glfwDestroyWindow(window);
    glfwTerminate();
}
