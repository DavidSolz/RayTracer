#ifndef RENDERINGCONTEXT_H
#define RENDERINGCONTEXT_H

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <vector>

#include "Vector3.h"
#include "Sphere.h"

struct RenderingContext {
    
    int width;
    int height;
    int depth; 

    std::vector<Sphere> spheres;
    Vector3 cameraPos;
    Sphere sun;

    GLuint pbo;
    GLuint texture;
    GLuint fboId;

};

#endif
