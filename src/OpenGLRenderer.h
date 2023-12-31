#ifndef OPENGLRENDERER_H
#define OPENGLRENDERER_H

#include <stdio.h>
#include <unordered_map>

#include "IFrameRender.h"
#include "Timer.h"

#include "ThreadedRendering.h"
#include "ParallelRendering.h"

class OpenGLRenderer {

private:

    enum {
        CPU,
        ACC
    } selection;

    IFrameRender * renderingService;
    GLFWwindow * window;

    Color* pixels;

    ThreadedRendering cpuRender;
    ParallelRendering gpuRender;

    float lastMouseX;
    float lastMouseY;

    void ProcessInput();

    static void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

public:
    OpenGLRenderer(RenderingContext * _context, const bool& _enableVSync = true);
    bool ShouldClose();
    void SetRenderingService(IFrameRender * _service);
    void Update();
    ~OpenGLRenderer();
};

#endif
