#ifndef RENDERINGCONTEXT_H
#define RENDERINGCONTEXT_H

#include "Object.h"
#include "Logger.h"
#include "Camera.h"
#include "BoundingBox.h"
#include "Texture.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>

struct RenderingContext {

    // Window settings
    uint32_t width = 640 ;
    uint32_t height = 480;
    uint32_t depth = 480;

    bool vSync = false;
    bool memorySharing = false;
    bool bvhAcceleration = false;
    bool boundedFrames = false;
    bool useCPU = false;

    // Texture transfer object
    GLuint textureID;
    
    // Objects data
    std::vector<Object> objects;
    std::vector<Material> materials;
    
    // Bounding Boxes
    std::vector<BoundingBox> boxes;

    // Texture data
    std::vector<Texture> textureInfo;
    std::vector<unsigned int> textureData;
    
    // Camera info
    Camera camera;
    uint32_t frameCounter = 0;

    uint32_t numBoundedFrames;
    uint32_t numThreads;
    float gamma = 2.2f;

    // Logging service
    Logger loggingService;

};

#endif
