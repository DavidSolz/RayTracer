#include "Configurator.h"

Configurator::Configurator(RenderingContext * _context){
    this->context = _context;

    context->width = 1000;
    context->height = 1000;
    context->depth = 480;

    context->camera.position = Vector3(context->width/2.0f, context->height/2.0f, -900.0f);
    context->camera.aspectRatio = context->width/(float)context->height;

    context->loggingService.BindOutput("RayTracer_log.txt");

}

void Configurator::ShowHelp(){

    fprintf(stdout, "Usage: RayTracer [options]\n");
    fprintf(stdout,"Options:\n");
    fprintf(stdout,"  -V              Enable VSync\n");
    fprintf(stdout,"  -L <filepath>   Load scene from specified file\n");
    fprintf(stdout,"  -w <width>      Set window width\n");
    fprintf(stdout,"  -h <height>     Set window height\n");
    fprintf(stdout,"  -S              Enable memory sharing\n");
    fprintf(stdout,"  -H              Show this help menu\n");

}


void Configurator::ParseArgs(const size_t & size, char **args){

    const char* sceneFile = nullptr;

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
                sceneFile = args[i + 1];
                i++;
            } else {
                fprintf(stderr, "Error: -L flag requires a scene file path\n");
                return;
            }
        } else if (arg[1] == 'w' && arg[2] == '\0') {
            if (i + 1 < size && args[i + 1][0] != '-') {
                context->width = atoi(args[i+1]);
                i++;
            } else {
                fprintf(stderr, "Error: -w flag requires window width\n");
                return;
            }
        }else if (arg[1] == 'h' && arg[2] == '\0') {
            if (i + 1 < size && args[i + 1][0] != '-') {
                context->height = atoi(args[i+1]);
                i++;
            } else {
                fprintf(stderr, "Error: -h flag requires requires window height\n");
                return;
            }
        }else if (arg[1] == 'S' && arg[2] == '\0') {
            fprintf(stdout, "Memory sharing enabled.\n");
            context->memorySharing = true;
        } else if (arg[1] == 'H' && arg[2] == '\0') {
            ShowHelp();
            exit(-1);
        }else {
            fprintf(stderr, "Unknown argument: %s\n", arg);
            exit(-1);
        }
    }

    context->camera.aspectRatio = context->width/(float)context->height;

}
