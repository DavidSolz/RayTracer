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
    
}

Configurator::~Configurator(){
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

}

void Configurator::ParseArgs(const size_t & size, char **args){

    const char * filepath = nullptr;

    for (int i = 1; i < size; ++i) {
        const char* arg = args[i];

        if(arg[0] != '-'){
            fprintf(stderr, "Unknown argument: %s\n", arg);
            return;
        }else if (arg[1] == 'V' && arg[2] == '\0') {
            fprintf(stdout, "VSync enabled.\n");
            context->vSync = true;
        } else if (arg[1] == 'L' && arg[2] == '\0') {
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
        }else if (arg[1] == 'S' && arg[2] == '\0') {
            fprintf(stdout, "Memory sharing enabled.\n");
            context->memorySharing = true;
        } else if (arg[1] == 'B' && arg[2] == '\0') {
            fprintf(stdout, "BVH tree enabled.\n");
            context->bvhAcceleration = true;
        } else if (arg[1] == 'H' && arg[2] == '\0') {
            ShowHelp();
            exit(0);
        }
    }

    Material material;
    
    material.albedo = {1.0f, 1.0f, 1.0f, 1.0f};
    material.tint = {1.0f,1.0f,1.0f,1.0f};
    material.specular = {1.0f, 1.0f, 1.0f, 1.0f};
    material.specularIntensity = 1.0f;
    material.indexOfRefraction = 1.0f; 
    material.roughness = 0.5f;
    material.tintRoughness = 0.5f;
    material.textureID = 0;

    context->materials.emplace_back(material);

    Texture info;

    info.width = 1;
    info.height = 1;
    info.offset = 0;

    context->textureInfo.emplace_back(info);
    context->textureData.emplace_back(UINT32_MAX);

    context->camera.aspectRatio = context->width/(float)context->height;

    if( filepath != NULL )
        serializer->LoadFromFile(filepath);

}
