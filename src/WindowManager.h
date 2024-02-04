#ifndef OPENGLRENDERER_H
#define OPENGLRENDERER_H

#include <stdio.h>
#include <unordered_map>

#include "IFrameRender.h"
#include "ILog.h"
#include "Timer.h"

#include "ThreadedRendering.h"
#include "ParallelRendering.h"

class WindowManager {

private:

    enum {
        CPU,
        ACC
    };

    IFrameRender ** renderingServices;
    uint32_t servicesSize;
    uint32_t selectedService;

    char windowTitle[50]={0};
    GLFWwindow * window;

    Color* pixels;

    double lastMouseX;
    double lastMouseY;

    void ProcessInput();

    void TakeScreenShot();

    void HandleErrors();

    static void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

public:
    WindowManager(RenderingContext * _context, const bool & _enableVSync = true);

    bool ShouldClose();

    void BindRenderingServices(IFrameRender * _services[], const uint32_t & _size);

    void SetDefaultRendering(const uint32_t & _index);

    void Update();
    
    ~WindowManager();
};

#endif
