#ifndef RENDERINGCONTEXT_H
#define RENDERINGCONTEXT_H

#define GL_SILENCE_DEPRECATION

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Mesh.h"
#include "Object.h"
#include "Logger.h"
#include "Camera.h"
#include "Texture.h"

struct RenderingContext {

    // Window settings
    uint32_t width = 640 ;
    uint32_t height = 480;
    uint32_t depth = 480;

    bool vSync = false;
    bool memorySharing = false;

    // Texture transfer object
    GLuint textureID;
    
    // Objects data
    std::vector<Object> objects;
    std::vector<Material> materials;
    std::vector<Texture> textureInfo;

    Mesh mesh;

    // Texture data
    std::vector<int> textureData;
    std::vector<int> normalMap;
    
    // Camera info
    Camera camera;
    uint32_t frameCounter;

    // Logging service
    Logger loggingService;

};

#endif
