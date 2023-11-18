#ifndef OPENGLRENDERER_H
#define OPENGLRENDERER_H

#include <stdio.h>

#include "IFrameRender.h"

#include "ThreadedRendering.h"
#include "GPUParallelRenderer.h"

class OpenGLRenderer {

private:

    enum {
        CPU,
        GPU
    } selection;

    IFrameRender * renderingService;
    RenderingContext * context;
    GLFWwindow * window;

    bool cpu = true;
    ThreadedRendering cpuRender;
    GPUParallelRenderer gpuRender;

    void ProcessInput();

public:
    OpenGLRenderer(RenderingContext * _context);
    bool ShouldClose();
    void SetRenderingService(IFrameRender * _service);
    void Update();
    ~OpenGLRenderer();
};

#endif
