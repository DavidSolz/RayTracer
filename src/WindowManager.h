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

    IFrameRender * renderingServices[10];
    uint8_t selectedService;
    uint8_t minIndex, maxIndex;

    char windowTitle[50]={0};
    GLFWwindow * window;

    Color* pixels;

    double lastMouseX;
    double lastMouseY;

    void ProcessInput();

    void TakeScreenShot();

    void HandleErrors();

    void UpdateWindow();

    static void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

public:
    WindowManager(RenderingContext * _context);

    bool ShouldClose();

    void BindRenderingService(const uint8_t & _key, IFrameRender * _service);

    void Update();
    
    ~WindowManager();
};

#endif
