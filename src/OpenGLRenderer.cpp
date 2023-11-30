#include "OpenGLRenderer.h"

OpenGLRenderer::OpenGLRenderer(RenderingContext * _context){

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
    glfwSwapInterval(0);

    glOrtho(0, context->width, 0, context->height, 0, context->depth);

    if (glewInit() != GLEW_OK) {
        glfwDestroyWindow(window);
        glfwTerminate();
        fprintf(stderr, "Failed to initialize GLEW\n");
        return;
    }

    pixels = new Color[context->width*context->height];

    timer = Timer::GetInstance();

}


void OpenGLRenderer::ProcessInput(){

    std::unordered_map<int, Vector3> keyMappings = {
        {GLFW_KEY_R, context->camera.front},
        {GLFW_KEY_F, context->camera.front * -1.0f},
        {GLFW_KEY_A, context->camera.right *-1.0f},
        {GLFW_KEY_D, context->camera.right},
        {GLFW_KEY_W, context->camera.up},
        {GLFW_KEY_S, context->camera.up * -1.0f},

    };

    for (auto& [key, direction] : keyMappings) {
        if (glfwGetKey(window, key) == GLFW_PRESS) {

            /*
                TODO:
                -fix camera rotation
                
            */

            //context->camera.Rotate(10.0f);
            context->camera.position = context->camera.position + direction * context->camera.movementSpeed * timer->GetDeltaTime();

            context->frameCounter=0;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && selection != CPU){
        fprintf(stdout, "Switching to cpu context.\n");
        SetRenderingService(&cpuRender);
        selection = CPU;
    }

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && selection != ACC){
        fprintf(stdout, "Switching to gpu context.\n");
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
