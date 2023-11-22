#ifndef RENDERINGCONTEXT_H
#define RENDERINGCONTEXT_H


#ifdef __APPLE__
    #include <GL/glew.h>
#else
    #include <GL/glew.h>
#endif

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
    Camera camera;
    Sphere sun;

    unsigned int frameCounter;

    GLuint pbo;

};

#endif
