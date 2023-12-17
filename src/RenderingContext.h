#ifndef RENDERINGCONTEXT_H
#define RENDERINGCONTEXT_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>

#include "Vector3.h"
#include "Object.h"
#include "Camera.h"

struct RenderingContext {

    int width;
    int height;
    int depth;

    std::vector<Object> objects;
    std::vector<Material> materials;

    Camera camera;
    unsigned int frameCounter;


};

#endif
