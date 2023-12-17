#ifndef RENDERINGCONTEXT_H
#define RENDERINGCONTEXT_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>

#include "Vector3.h"
#include "Sphere.h"
#include "Camera.h"

struct RenderingContext {

    int width;
    int height;
    int depth;

    std::vector<Sphere> spheres;
    std::vector<Material> materials;

    Camera camera;
    Sphere sun;

    unsigned int frameCounter;


};

#endif
