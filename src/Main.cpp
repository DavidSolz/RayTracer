
#include "WindowManager.h"
#include "PerformanceMonitor.h"
#include "Configurator.h"

#include "ThreadedRendering.h"

#include "ThreadedShader.h"
#include "CLShader.h"

void HandleCameraMovement(RenderingContext & context, const Vector3 & direction);

void SetupKeyBindings(RenderingContext & context, WindowManager & manager, IFrameRender * accelerator, IFrameRender * processor);

int main(int argc, char **argv){

    // Objects setup

    RenderingContext context;

    BVHTree tree(&context);

    Configurator configurator(&context);

    configurator.ParseArgs(argc, argv);

    MaterialBuilder materialBuilder(&context);

// {

//     // DISK

//     Object p;

//     p.position = Vector3(context.width/2.0f, context.height/4.0f, context.depth/4.0f);
//     p.normal = Vector3(0.0f, 1.0f ,0.0f);
//     p.type = PLANE;

//     p.materialID = materialBuilder
//                     .SetBaseColor({0.5f, 0.5f, 0.5f, 1.0f})
//                     ->Build();

//     context.objects.emplace_back(p);

// // Light
//     p.position = Vector3(context.width/2.0f, 2*context.height, context.depth/4.0f);
//     p.normal = Vector3(0.0f, -1.0f ,0.0f);
//     p.radius = 300.0f;
//     p.type = DISK;

//     p.materialID = materialBuilder
//                     .SetBaseColor({1.0f, 1.0f, 1.0f, 1.0f})
//                     ->SetEmission(5.0f)
//                     ->Build();

//     context.objects.emplace_back(p);


//     const int rows = 10;
//     const int cols = 10;
//     const float spacing = 100.0f;

//     for (int i = 0; i < rows; i++) {
//         for (int j = 0; j < cols; j++) {
//             Object s;

//             //s.position = Vector3(context.width/2.0f, context.height/2.0f, context.depth/4.0f);
//             s.position.x = i * spacing;
//             s.position.z = context.height/2.0f;
//             s.position.y = context.height/2.0f + j * spacing;
//             s.radius = spacing/2.0f;
//             s.type = SPHERE;

//             float ti = (i)/(float)(rows);
//             float tj = (j)/(float)(cols);

//             float isEmissive = (rand() / (float)RAND_MAX)>0.4f;
//             float isMetallic = (rand() / (float)RAND_MAX)>0.8f;
//             float isGlass = (rand() / (float)RAND_MAX)>0.8f;

//             const Color colorA = (Color){1.0f, 0.0f, 0.0f, 1.0f};
//             const Color colorB = (Color){0.0f, 1.0f, 0.0f, 1.0f};
//             const Color colorC = (Color){0.0f, 0.0f, 1.0f, 1.0f};
//             Color color = Color::Lerp(colorA, colorB, ti);
//             color = Color::Lerp(color, colorC, tj);

//             //Color color = (Color){(rand() / (float)RAND_MAX),(rand() / (float)RAND_MAX),(rand() / (float)RAND_MAX), 1.0f};

//             s.materialID =  materialBuilder
//                             .SetBaseColor(color)
//                             ->SetDiffusionColor(color)
//                             ->SetSpecularColor(color)
//                             ->SetRefractiveIndex((rand() / (float)RAND_MAX) + 1.0f)
//                             ->SetSpecularIntensity((rand() / (float)RAND_MAX))
//                             ->SetSmoothness((rand() / (float)RAND_MAX) * isMetallic  * (1.0f - isGlass) * (1.0f - isEmissive))
//                             ->SetEmission( ( (rand()/ (float)RAND_MAX) ) * isEmissive * (1.0f - isMetallic) * (1.0f - isGlass))
//                             ->SetRoughness((rand() / (float)RAND_MAX))
//                             ->SetTransparency((rand() / (float)RAND_MAX) * isGlass * (1.0f - isMetallic) * (1.0f - isEmissive))
//                             ->Build();

//             context.objects.emplace_back(s);
//         }
//     }

// }



//{
//DISK

    Object p;

    p.position = Vector3(context.width/2.0f, context.height/4.0f, context.depth/4.0f);
    p.normal = Vector3(0.0f, 1.0f, 0.0f);
    p.maxPos = Vector3(5000.0f, 5000.0f, 5000.0f);
    p.radius = 1000.0f;
    p.type = DISK;

    p.materialID = materialBuilder
                    .SetBaseColor({0.5f, 0.5f, 0.5f, 1.0f})
                    ->SetRoughness(1.0f)
                    ->AttachTexture( "resources/textures/dunes.bmp" )
                    ->Build();

    context.objects.emplace_back(p);

// Light
    p.position = Vector3(context.width/2.0f, 2*context.height, context.depth/2.0f);
    p.normal = Vector3(0.0f, -1.0f, 0.0f);
    p.radius = 600.0f;
    p.type = DISK;

    p.materialID = materialBuilder
                    .SetBaseColor({1.0f, 1.0f, 0.8f, 1.0f})
                    ->SetEmission(2.0f)
                    ->Build();

    context.objects.emplace_back(p);

// // RED SPHERE
//     Vector3 pos = Vector3(200.0f, 200.0f, 200.0f);
//     p.type = CUBE;

//     p.position = Vector3(context.width/2.0f, context.height/4.0f, context.depth/2.0f);
//     p.maxPos = pos + p.position;


//     p.materialID = materialBuilder
//                     .SetBaseColor(1.0f, 0.0f, 0.0f)
//                     ->SetRoughness(0.7f)
//                     ->SetTransparency(1.0f)
//                     ->SetRefractiveIndex(1.72f)
//                     ->Build();

//     context.objects.emplace_back(p);    

//     p.position = Vector3(context.width/3.0f, context.height/4.0f + 100.0f, 0.0f);
//     p.radius = 100.0f;
//     p.type = SPHERE;
    
//     p.materialID = materialBuilder
//                 .SetEmission(0.1f)
//                 ->AttachTexture( "resources/textures/marble.bmp" )
//                 ->Build();

//     context.objects.emplace_back(p);

//     p.position = Vector3(4.5f*context.width/5.0f, context.height/4.0f, 0.0f);
//     p.maxPos = p.position + Vector3(400.0f, 400.0f, 400.0f);
//     p.type = CUBE;

//     p.materialID = materialBuilder
//                     .SetSmoothness(1.0f)
//                     ->SetRoughness(0.5f)
//                     ->AttachTexture( "resources/textures/metal.bmp" )
//                     ->Build();

//     context.objects.emplace_back(p);

// }

//Main loop

    tree.BuildBVH();

    WindowManager manager(&context);
    PerformanceMonitor monitor;

    ThreadedRendering processor(&context);

    ThreadedShader threaded(&context);
    CLShader accelerated(&context);

    manager.SetRenderingService(&accelerated, "Acc mode");

    SetupKeyBindings(context, manager, &accelerated, &processor);

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