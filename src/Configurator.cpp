#include "Configurator.h"

Configurator::Configurator(RenderingContext * _context){
    this->context = _context;

    context->width = 1000;
    context->height = 1000;
    context->depth = 480;

    context->camera.position = Vector3(context->width/2.0f, context->height/2.0f, -900.0f);
    context->camera.aspectRatio = context->width/(float)context->height;

    context->loggingService.BindOutput("RayTracer_log.txt");

    ComputeEnvironment::SetContext( _context );

    this->serializer = new SceneSerializer(context);
    this->tree = new BVHTree(context);

    Initialize();
}

void Configurator::Initialize(){
    Material material;
    
    material.albedo = {0.5f, 0.5f, 0.5f, 1.0f};
    material.tint = {};
    material.specular = {};
    material.emmissionIntensity = 0.0f;
    material.specularIntensity = 0.0f;
    material.indexOfRefraction = 1.45f; 
    material.roughness = 0.5f;
    material.tintRoughness = 0.5f;
    material.textureID = 0;

    context->materials.emplace_back(material);

    Texture info;

    info.width = 1;
    info.height = 1;
    info.offset = 0;
    info.checksum = UINT32_MAX;

    context->textureInfo.emplace_back(info);
    context->textureData.emplace_back(UINT32_MAX);

}

Configurator::~Configurator(){
    delete tree;
    delete serializer;
}

void Configurator::ShowHelp(){

    fprintf(stdout, "Usage: RayTracer [options]\n");
    fprintf(stdout,"Options:\n");
    fprintf(stdout,"  -V              Enable VSync\n");
    fprintf(stdout,"  -L <filepath>   Load scene from file\n");
    fprintf(stdout,"  -w <width>      Set window width\n");
    fprintf(stdout,"  -h <height>     Set window height\n");
    fprintf(stdout,"  -S              Enable memory sharing\n");
    fprintf(stdout,"  -H              Show help menu\n");
    fprintf(stdout,"  -B              Build BVH tree\n");
    fprintf(stdout,"  -T <threads>    Set number of threads\n");
    fprintf(stdout,"  -F <frames>     Set number of frames to render\n");

}

void Configurator::ParseArgs(const size_t & size, char **args){

    const char * filepath = NULL;

    context->numThreads = std::thread::hardware_concurrency();

    for (int i = 1; i < size; ++i) {
        const char* arg = args[i];

        if(arg[0] != '-'){
            fprintf(stderr, "Unknown argument: %s\n", arg);
            exit(-1);
        }else if (arg[1] == 'V' && arg[2] == '\0' && context->vSync == false) {
            fprintf(stdout, "VSync enabled.\n");
            context->vSync = true;
        } else if (arg[1] == 'L' && arg[2] == '\0' && filepath == NULL) {
            if (i + 1 < size && args[i + 1][0] != '-') {
                filepath = args[i + 1];
                i++;
            } else {
                fprintf(stderr, "Error: -L flag requires a scene file path\n");
                exit(-1);
            }
        } else if (arg[1] == 'w' && arg[2] == '\0') {
            if (i + 1 < size && args[i + 1][0] != '-') {
                context->width = std::max(atoi(args[i+1]), 100);
                i++;
            } else {
                fprintf(stderr, "Error: -w flag requires window width\n");
                exit(-1);
            }
        }else if (arg[1] == 'h' && arg[2] == '\0') {
            if (i + 1 < size && args[i + 1][0] != '-') {
                context->height = std::max(atoi(args[i+1]), 100);
                i++;
            } else {
                fprintf(stderr, "Error: -h flag requires requires window height\n");
                exit(-1);
            }
        }else if (arg[1] == 'T' && arg[2] == '\0') {
            if (i + 1 < size && args[i + 1][0] != '-') {
                context->numThreads = std::max(atoi(args[i+1]), 1);
                i++;
            } else {
                fprintf(stderr, "Error: -T flag requires requires number of threads\n");
                exit(-1);
            }
        }else if (arg[1] == 'F' && arg[2] == '\0') {
            if (i + 1 < size && args[i + 1][0] != '-') {
                context->numBoundedFrames = std::max(atoi(args[i+1]), 1);
                context->boundedFrames = true;
                i++;
            } else {
                fprintf(stderr, "Error: -F flag requires requires number of frames\n");
                exit(-1);
            }
        }else if (arg[1] == 'S' && arg[2] == '\0' && context->memorySharing == false) {
            fprintf(stdout, "Memory sharing enabled.\n");
            context->memorySharing = true;
        } else if (arg[1] == 'B' && arg[2] == '\0' && context->bvhAcceleration == false) {
            fprintf(stdout, "BVH tree enabled.\n");
            context->bvhAcceleration = true;
        } else if (arg[1] == 'H' && arg[2] == '\0') {
            ShowHelp();
            exit(0);
        }
    }

    context->width = std::max((uint32_t)32, (context->width+16)/32 * 32);
    context->height = std::max((uint32_t)32, (context->height+16)/32 * 32);

    printf("Resolution : %d x %d\n", context->width, context->height);

    context->camera.aspectRatio = context->width/(float)context->height;

    if( filepath != NULL )
        serializer->LoadFromFile(filepath);
        
    if( context->bvhAcceleration == true )
        tree->BuildBVH();

}
