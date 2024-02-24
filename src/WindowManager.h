#ifndef OPENGLRENDERER_H
#define OPENGLRENDERER_H

#include <stdio.h>

#include "IFrameRender.h"
#include "InputService.h"
#include "Timer.h"


class WindowManager {

private:

    IFrameRender * renderingServices[10];
    uint8_t selectedService;
    uint8_t minIndex, maxIndex;

    char windowTitle[50]={0};
    GLFWwindow * window;

    InputService * input;

    Color * pixels;
    
    double lastMouseX;
    double lastMouseY;

    void ProcessInput();

    void TakeScreenShot();

    void HandleErrors();

    void UpdateWindow();

public:
    WindowManager(RenderingContext * _context);

    bool ShouldClose();

    void BindRenderingService(const uint8_t & _key, IFrameRender * _service);

    void Update();
    
    ~WindowManager();
};

#endif
