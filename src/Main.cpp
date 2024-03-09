
#include "WindowManager.h"
#include "PerformanceMonitor.h"
#include "Configurator.h"

#include "ThreadedShader.h"
#include "CLShader.h"

void HandleCameraMovement(RenderingContext & context, const Vector3 & direction);

void SetupKeyBindings(RenderingContext & context, WindowManager & manager, IFrameRender * accelerator, IFrameRender * processor);

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

    // Key binding and service setup

    manager.SetRenderingService(&accelerated, "Acc mode");
    SetupKeyBindings(context, manager, &accelerated, &threaded);

    // Main loop

    while ( manager.ShouldClose() ) {
        manager.Update();
        monitor.GatherInformation();
    }

    return EXIT_SUCCESS;
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