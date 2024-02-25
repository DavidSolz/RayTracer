#include "WindowManager.h"

static RenderingContext * context;
static Timer * timer;

WindowManager::WindowManager(RenderingContext * _context){

    context = _context;

    context->loggingService.Write(MessageType::INFO, "Configuring window...");

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
    glfwSwapInterval( context->vSync );

    glLoadIdentity();
    glOrtho(0, context->width, 0, context->height, -1.0f, 1.0f);

    if (glewInit() != GLEW_OK) {
        glfwDestroyWindow(window);
        glfwTerminate();

        context->loggingService.Write(MessageType::INFO, "Failed to initialize GLEW");
        context->loggingService.Write(MessageType::INFO, "Window configuration done");

        return;
    }

    context->loggingService.Write(MessageType::INFO, "Creating texture buffer...");

    glGenTextures(1, &context->textureID);
    glBindTexture(GL_TEXTURE_2D, context->textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, context->width, context->height, 0, GL_RGBA, GL_FLOAT, nullptr);

    context->loggingService.Write(MessageType::INFO, "Current resolution : %d x %d", context->width, context->height);

    context->loggingService.Write(MessageType::INFO, "Checking for V-Sync : %s", context->vSync?"enabled":"disabled");

    context->loggingService.Write(MessageType::INFO, "Window configuration done");

    pixels = new Color[context->width * context->height];

    timer = &Timer::GetInstance();

}

void WindowManager::ProcessInput(){

    for(auto pair : actions){

        if( glfwGetKey(window, pair.first) == GLFW_PRESS)
            pair.second();

    }

    if (glfwGetMouseButton(window,GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        
        static double lastMouseX, lastMouseY;
        double currentX, currentY;

        glfwGetCursorPos(window, &currentX, &currentY);

        if ( currentX == lastMouseX and currentY == lastMouseY)
            return;

        double offsetX = lastMouseX - currentX;
        double offsetY = lastMouseY - currentY;

        double & deltaTime = timer->GetDeltaFrame();

        double len = sqrt(offsetX*offsetX + offsetY*offsetY);
        offsetX = (offsetX * deltaTime)/len;
        offsetY = (offsetY * deltaTime)/len;

        context->camera.Rotate(offsetX, offsetY);

        lastMouseX = currentX;
        lastMouseY = currentY;

        context->frameCounter=0;
    }
    
}

void WindowManager::SetWindowTitle(const char _title[9]){
    memcpy(windowTitle, _title, 9);
}

bool WindowManager::ShouldClose(){
    return !glfwWindowShouldClose(window);
}

void WindowManager::BindAction(const uint16_t & _key, Callback _callback){
    actions[ _key ] = _callback;
}

void  WindowManager::HandleErrors(){

    GLenum error = glGetError();
    if(error!=GL_NO_ERROR){
        context->loggingService.Write(MessageType::ISSUE, "OpenGL buffer error");
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    }

    if( renderer == nullptr ){
        context->loggingService.Write(MessageType::ISSUE, "Rendering service error");
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void WindowManager::UpdateWindow(){

    timer->TicTac();

    renderer->Render(pixels);

    context->frameCounter++;

    if( !context->memorySharing )
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, context->width, context->height, GL_RGBA, GL_FLOAT, pixels);

    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
         glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
         glTexCoord2f(1.0f, 0.0f); glVertex2f(context->width, 0.0f);
         glTexCoord2f(1.0f, 1.0f); glVertex2f(context->width, context->height);
         glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, context->height);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    if( timer->GetAccumulatedTime() >= 1.0f){
        uint32_t & fps = timer->GetFrameCount();
        double & deltaTime = timer->GetDeltaTime();
        sprintf(windowTitle+8, " | FPS : %d | Frametime : %5.3f ms\0", fps, deltaTime);
        glfwSetWindowTitle(window, windowTitle);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
}

void WindowManager::Update(){

    ProcessInput();
    
    HandleErrors();

    UpdateWindow();

}

void WindowManager::SetRenderingService(IFrameRender * _service, const char _name[9]){
    this->renderer = _service;
    memcpy(windowTitle, _name, 9);
}

bool WindowManager::IsButtonPressed(const uint16_t & _key){
    return glfwGetKey(window, _key);
}

void WindowManager::Close(){
    glfwSetWindowShouldClose(window, true);
}

void WindowManager::DumpContent(){

    glReadPixels(0, 0, context->width, context->height, GL_RGBA, GL_FLOAT, pixels);

    std::ofstream outFile("screenshot.bmp", std::ios::binary);

    outFile << "BM"; 
    uint32_t fileSize = 54 + sizeof(Color) * context->width * context->height; 
    outFile.write((char*)(&fileSize), 4);
    outFile.write("\0\0\0\0", 4); 
    outFile.write("\x36\0\0\0", 4); 

    outFile.write("\x28\0\0\0", 4); 
    outFile.write((char*)(&context->width), 4); 
    outFile.write((char*)(&context->height), 4); 
    outFile.write("\x01\0", 2);
    outFile.write("\x20\0", 2); 
    outFile.write("\0\0\0\0", 4);
    outFile.write((char*)(&fileSize), 4);
    outFile.write("\x13\x0B\0\0", 4); 
    outFile.write("\x13\x0B\0\0", 4);
    outFile.write("\0\0\0\0", 4); 
    outFile.write("\0\0\0\0", 4); 

    for (int y = context->height - 1; y >= 0; --y) {
        for (int x = 0; x < context->width; ++x) {

            const Color& pixel = pixels[( context->height - 1 - y) * context->width + x];
            
            uint8_t bgra[4] = { 
                uint8_t(pixel.B * 255), 
                uint8_t(pixel.G * 255), 
                uint8_t(pixel.R * 255), 
                uint8_t(pixel.A * 255)
            };
                
            outFile.write((char*)(bgra), 4);
            
        }
    }

    outFile.close();
}

WindowManager::~WindowManager(){

    delete[] pixels;

    context->loggingService.Write(MessageType::INFO, "Deleting texture buffer...");

    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &context->textureID);
    
    context->loggingService.Write(MessageType::INFO, "Destroying window...");

    glfwDestroyWindow(window);
    glfwTerminate();

}
