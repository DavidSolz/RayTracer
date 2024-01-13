#ifndef RENDERINGCONTEXT_H
#define RENDERINGCONTEXT_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>

#include "Vector3.h"
#include "Object.h"
#include "Camera.h"

struct RenderingContext {

    uint32_t width;
    uint32_t height;
    uint32_t depth;

    std::vector<Object> objects;
    std::vector<Material> materials;

    Camera camera;
    uint32_t frameCounter;


};

#endif
