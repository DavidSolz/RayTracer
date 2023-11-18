#include "OpenGLRenderer.h"

OpenGLRenderer::OpenGLRenderer(RenderingContext * _context){
    
    this->context = _context;
    
    if(!glfwInit()){
        printf("Cannot initialize GLFW \n");
        return;
    }
    
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);


    window = glfwCreateWindow(context->width, context->height, "RayTracer", NULL, NULL);

    if(!window){
        glfwTerminate();
        printf("Cannot create GLFW window\n");
        return;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glOrtho(0, context->width, 0, context->height, 0, context->depth);


    if (glewInit() != GLEW_OK) {
        glfwDestroyWindow(window);
        glfwTerminate();
        fprintf(stderr, "Failed to initialize GLEW\n");
        return;
    }

    glGenBuffers(1, &context->pbo);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, context->pbo);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, context->width * context->height * sizeof(Color), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    cpuRender.Init(context);
    gpuRender.Init(context);
    SetRenderingService(&gpuRender);
}


void OpenGLRenderer::ProcessInput(){
    Vector3 cameraFront = {0.0f, 0.0f, -1.0f};
    Vector3 cameraUp = {0.0f, 1.0f, 0.0f};

    float cameraSpeed = 10.0f;
    
    std::unordered_map<int, Vector3> keyMappings = {
        {GLFW_KEY_R, cameraFront},
        {GLFW_KEY_F, cameraFront * -1.0f},
        {GLFW_KEY_D, Vector3::CrossProduct(cameraFront, cameraUp).Normalize() * -1.0f},
        {GLFW_KEY_A, Vector3::CrossProduct(cameraFront, cameraUp).Normalize() },
        {GLFW_KEY_W, cameraUp * -1.0f},
        {GLFW_KEY_S, cameraUp}
    };

    for (auto& [key, direction] : keyMappings) {
        if (glfwGetKey(window, key) == GLFW_PRESS) {
            context->cameraPos = context->cameraPos + direction * cameraSpeed;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && selection != CPU){
        fprintf(stdout, "Switching to cpu context.\n");
        SetRenderingService(&cpuRender);
        selection = CPU;
    }

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && selection != GPU){
        fprintf(stdout, "Switching to gpu context.\n");
        SetRenderingService(&gpuRender);
        selection = GPU;
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

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, context->pbo);
    Color * pixels = static_cast<Color*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY));

    renderingService->Render(pixels);

    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    glDrawPixels(context->width, context->height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glfwSwapBuffers(window);
    glfwPollEvents();

    GLenum error = glGetError();
    if(error!=GL_NO_ERROR)
        printf("Error : %d\n", error);
}

OpenGLRenderer::~OpenGLRenderer(){
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDeleteBuffers(1, &context->pbo);
    glDeleteTextures(1, &context->texture);

    glfwDestroyWindow(window);
    glfwTerminate();
}