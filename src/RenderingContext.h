#ifndef RENDERINGCONTEXT_H
#define RENDERINGCONTEXT_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Mesh.h"
#include "Object.h"
#include "Logger.h"
#include "Camera.h"

static GLuint textureID;

struct RenderingContext {

    // Window settings
    uint32_t width;
    uint32_t height;
    uint32_t depth;

    // Objects data
    std::vector<Object> objects;
    std::vector<Material> materials;
    Mesh mesh;

    // Camera info
    Camera camera;
    uint32_t frameCounter;

    // Logging service
    Logger loggingService;

};

#endif
