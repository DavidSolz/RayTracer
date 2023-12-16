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
    RenderingContext * context;
    GLFWwindow * window;

    Color* pixels;
    Timer* timer;

    ThreadedRendering cpuRender;
    ParallelRendering gpuRender;

    void ProcessInput();

public:
    OpenGLRenderer(RenderingContext * _context, const bool& _enableVSync);
    bool ShouldClose();
    void SetRenderingService(IFrameRender * _service);
    void Update();
    ~OpenGLRenderer();
};

#endif
