
#include "WindowManager.h"
#include "PerformanceMonitor.h"
#include "Configurator.h"

#include "ThreadedShader.h"
#include "CLShader.h"

void HandleCameraMovement(RenderingContext & context, const Vector3 & direction);

void SetupKeyBindings(RenderingContext & context, WindowManager & manager);

int main(int argc, char **argv){

    // Context setup

    RenderingContext context;

    // Argument parsing

    Configurator configurator(&context);
    configurator.ParseArgs(argc, argv);

    // Window and monitor setup

    WindowManager manager(&context);

    // Service setup

    IFrameRender * service;
    const char * name;

    if(context.useCPU){
        service = new ThreadedShader(&context);
        name = "CPU mode";
    }else{
        service = new CLShader(&context);
        name = "ACC mode";
    }

    manager.SetRenderingService(service, name);
    PerformanceMonitor monitor;

    // Main loop

    if( context.boundedFrames ){

        for(uint32_t frame = 0; frame < context.numBoundedFrames; ++frame){
            manager.Render();
            monitor.GatherInformation();
        }

        return EXIT_SUCCESS;
    }

    if( context.followCenter ){

        float yaw = 0.0f;
        float pitch = 0.0f;
        float radius = 5000.0f;

        const Vector3 center = Vector3(0.0f, 0.0f, 0.0f);

        while ( manager.ShouldClose() ) {
            manager.Update();
            monitor.GatherInformation();

            float yawInRad = yaw * deg2rad;
            float pitchInRad = pitch * deg2rad;

            float xNew = radius * sin(yawInRad);
            float yNew = 500.0f * cos(pitchInRad) + 500.0f;
            float zNew = radius * cos(yawInRad);

            context.frameCounter = 0;
            context.camera.position = Vector3(xNew, yNew, zNew);
            context.camera.LookAt(center);

            yaw = (yaw >= 360.0f) * (yaw - 360.0f) + (yaw < 360.0f) * yaw;
            yaw += 0.1f;
            pitch = (pitch >= 360.0f) * (pitch - 360.0f) + (pitch < 360.0f) * pitch;
            pitch += 0.1f;

        }

        return EXIT_SUCCESS;

    }

    SetupKeyBindings(context, manager);

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

void SetupKeyBindings(RenderingContext & context, WindowManager & manager){

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
