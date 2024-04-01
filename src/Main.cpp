
#include "WindowManager.h"
#include "PerformanceMonitor.h"
#include "Configurator.h"

#include "ThreadedShader.h"
#include "CLShader.h"

void HandleCameraMovement(RenderingContext & context, const Vector3 & direction);

void SetupKeyBindings(RenderingContext & context, WindowManager & manager, IFrameRender * accelerator, IFrameRender * processor);

void ShowProgress(float progress);

int main(int argc, char **argv){

    // Objects setup

    RenderingContext context;

    // Argument parsing

    Configurator configurator(&context);

    configurator.ParseArgs(argc, argv);

    // Window and monitor setup

    WindowManager manager(&context);
    PerformanceMonitor monitor;

    // Shader setup

    ThreadedShader threaded(&context);
    CLShader accelerated(&context);

    
    // Main loop

    if( context.boundedFrames ){

        if(context.useCPU){
            manager.SetRenderingService(&threaded, "CPU mode");
        }else{
            manager.SetRenderingService(&accelerated, "ACC mode");
        }
        
        for(uint32_t frame = 0; frame < context.numBoundedFrames; ++frame){
            manager.Render();
            monitor.GatherInformation();
            //ShowProgress(frame/((float)context.numBoundedFrames-1));
        }

        return EXIT_SUCCESS;
    }

    manager.SetRenderingService(&accelerated, "ACC mode");
    SetupKeyBindings(context, manager, &accelerated, &threaded);

    while ( manager.ShouldClose() ) {
        manager.Update();
        monitor.GatherInformation();
    }
        
    return EXIT_SUCCESS;
}

void ShowProgress(float progress) {
    int barWidth = 70;

    const char * tokens = " =";

    fprintf(stdout, "[");
    int pos = barWidth * progress;
    for (int i = 0; i < barWidth; ++i) {
        bool isBefore = i < pos;
        fprintf(stdout, "%c", tokens[isBefore] );
    }
    fprintf(stdout, "] %.1f %\r", progress * 100.0f);
    fflush(stdout);
}

void HandleCameraMovement(RenderingContext & context, const Vector3 & direction) {
    Timer& timer = Timer::GetInstance();
    context.camera.Move(direction, timer.GetDeltaFrame());
    context.frameCounter = 0;
}

void SetupKeyBindings(RenderingContext & context, WindowManager & manager, IFrameRender * accelerator, IFrameRender * processor){
    manager.BindAction(GLFW_KEY_0, [&manager, &processor](){
        manager.SetRenderingService(processor, "Cpu mode");
    });

    manager.BindAction(GLFW_KEY_1, [&manager, &accelerator](){
        manager.SetRenderingService(accelerator, "Acc mode");
    });

    manager.BindAction(GLFW_KEY_ESCAPE, [&manager](){
        manager.Close();
    });

    manager.BindAction(GLFW_KEY_E, [&manager](){
        manager.DumpContent();
    });

    manager.BindAction(GLFW_KEY_W, [&context](){
        HandleCameraMovement(context, context.camera.front);
    });

    manager.BindAction(GLFW_KEY_S, [&context](){
        HandleCameraMovement(context, context.camera.front*(-1.0f));
    });

    manager.BindAction(GLFW_KEY_A, [&context](){
        HandleCameraMovement(context, context.camera.right*(-1.0f));
    });

    manager.BindAction(GLFW_KEY_D, [&context](){
        HandleCameraMovement(context, context.camera.right);
    });

    manager.BindAction(GLFW_KEY_SPACE, [&context](){
        HandleCameraMovement(context, worldUp);
    });

    manager.BindAction(GLFW_KEY_LEFT_SHIFT, [&context](){
        HandleCameraMovement(context, worldUp * (-1.0f));
    });
}