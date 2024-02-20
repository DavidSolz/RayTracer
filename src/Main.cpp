
#include "WindowManager.h"
#include "MaterialBuilder.h"
#include "PerformanceMonitor.h"
#include "Configurator.h"
#include "MeshReader.h"

int main(int argc, char **argv){

    srand(time(NULL));

    // Objects setup

    RenderingContext context;

    Configurator configurator(&context);

    configurator.ParseArgs(argc, argv);

    MaterialBuilder materialBuilder(&context);


// {

//     Object p;

// // BLUE CUBE

//     p.position = Vector3(context.width/2.0f, context.height/4.0f, context.depth);
//     p.maxPos = p.position + Vector3(300, 400, 300);
//     p.type = CUBE;

//     p.materialID = materialBuilder
//                     .SetBaseColor({0.0f, 0.0f, 1.0f, 1.0f})
//                     ->Build();

//     context.objects.emplace_back(p);

// // RED CUBE

//     p.position = Vector3(context.width/3.0f, context.height/4.0f, context.depth/2.0f);
//     p.maxPos = p.position + Vector3(200, 200, 200);
//     p.type = CUBE;

//     p.materialID = materialBuilder
//                     .SetBaseColor({1.0f, 0.0f, 0.0f, 1.0f})
//                     ->Build();

//     context.objects.emplace_back(p);

// // GREEN CUBE

//     p.position = Vector3(context.width/5.0f, context.height/4.0f, 3*context.depth/4.0f);
//     p.maxPos = p.position + Vector3(100, 100, 100);
//     p.type = CUBE;

//     p.materialID = materialBuilder
//                     .SetBaseColor({0.0f, 1.0f, 0.0f, 1.0f})
//                     ->Build();

//     context.objects.emplace_back(p);

// // GLASS CUBE

//     p.position = Vector3(context.width/3.0f, context.height/4.0f, -130.0f);
//     p.maxPos = p.position + Vector3(300, 300, 20);
//     p.type = CUBE;

//     p.materialID = materialBuilder
//                     .SetBaseColor({1.0f, 1.0f, 1.0f, 1.0f})
//                     ->SetRefractiveIndex(2.25f)
//                     ->SetTransparency(1.0f)
//                     ->Build();

//     context.objects.emplace_back(p);

// // // MIRROR

// //     p.position = Vector3(context.width/2.0f, context.height/4.0f + 50.0f, 0.0f);
// //     p.type = SPHERE;
// //     p.radius = 50.0f;

// //     p.materialID = materialBuilder
// //                     .SetBaseColor({0.5f, 0.5f, 0.5f, 1.0f})
// //                     ->SetSmoothness(1.0f)
// //                     ->Build();

// //     context.objects.emplace_back(p);

// //     p.position = Vector3(context.width/2.0f + 120.0f, context.height/4.0f + 50.0f, 0.0f);
// //     p.type = SPHERE;
// //     p.radius = 50.0f;

// //     p.materialID = materialBuilder
// //                     .SetBaseColor({0.5f, 0.5f, 0.5f, 1.0f})
// //                     ->SetSmoothness(0.5f)
// //                     ->Build();

// //     context.objects.emplace_back(p);

// //     p.position = Vector3(context.width/2.0f + 240.0f, context.height/4.0f + 50.0f, 0.0f);
// //     p.type = SPHERE;
// //     p.radius = 50.0f;

// //     p.materialID = materialBuilder
// //                     .SetBaseColor({0.5f, 0.5f, 0.5f, 1.0f})
// //                     ->SetSmoothness(0.0f)
// //                     ->Build();

// //     context.objects.emplace_back(p);


// // PLANE

//     p.position = Vector3(context.width/2.0f, context.height/4.0f, context.depth/4.0f);
//     p.normal = Vector3(0.0f, 1.0f ,0.0f);
//     p.radius = 1000.0f;
//     p.type = DISK;

//     p.materialID = materialBuilder
//                     .SetBaseColor({0.5f, 0.5f, 0.5f, 1.0f})
//                     ->SetRoughness(0.5f)
//                     ->Build();

//     context.objects.emplace_back(p);

// // SUN
//     p.position = Vector3(context.width/2.0f, 2*context.height, context.depth/4.0f);
//     p.radius = 600.0f;
//     p.type = SPHERE;

//     p.materialID = materialBuilder
//                     .SetEmissionColor({1.0f, 1.0f, 1.0f, 1.0f})
//                     ->SetEmission(10.0f)
//                     ->Build();

//     context.objects.emplace_back(p);

// }

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



{
// DISK

    Object p;

    p.position = Vector3(context.width/2.0f, context.height/4.0f, context.depth/4.0f);
    p.normal = Vector3(0.0f, 1.0f, 0.0f);
    p.maxPos = Vector3(5000.0f, 5000.0f, 5000.0f);
    p.radius = 1000.0f;
    p.type = PLANE;

    p.materialID = materialBuilder
                    .SetBaseColor({0.5f, 0.5f, 0.5f, 1.0f})
                    ->SetRoughness(1.0f)
                    ->AttachTexture( "resources/textures/sand.bmp" )
                    ->Build();

    context.objects.emplace_back(p);

// Light
    p.position = Vector3(context.width/2.0f, 4*context.height, context.depth/4.0f);
    p.normal = Vector3(0.0f, -1.0f ,0.0f);
    p.radius = 300.0f;
    p.type = DISK;

    p.materialID = materialBuilder
                    .SetBaseColor({1.0f, 1.0f, 1.0f, 1.0f})
                    ->SetEmission(2.0f)
                    ->Build();

    context.objects.emplace_back(p);

// RED SPHERE
    p.position = Vector3(context.width/2.0f, context.height/4.0f + 100.0f, context.depth/2.0f);
    p.radius = 100.0f;
    p.type = SPHERE;

    p.materialID = materialBuilder
                    .AttachTexture( "resources/textures/crystal.bmp" )
                    ->Build();

    context.objects.emplace_back(p);

    p.position = Vector3(context.width/4.0f, context.height/4.0f + 100.0f, 0.0f);
    p.radius = 100.0f;
    p.type = SPHERE;
    
    p.materialID = materialBuilder
                .SetBaseColor((Color){0.3f, 0.4f, 0.35f, 1.0f})
                ->SetSmoothness(1.0f)
                ->SetRoughness(0.5f)
                ->Build();

    context.objects.emplace_back(p);

    p.position = Vector3(4.5*context.width/5.0f, context.height/4.0f, 0.0f);
    p.maxPos = p.position + Vector3(400.0f, 400.0f, 400.0f);
    p.type = CUBE;

    p.materialID = materialBuilder
                    .SetSmoothness(1.0f)
                    ->SetRoughness(0.0f)
                    ->AttachTexture( "resources/textures/metal.bmp" )
                    ->Build();

    context.objects.emplace_back(p);


    

// Transfer mesh into context

    // MeshReader reader(&context);

    // Mesh * mesh = reader.LoadObject("resources/mesh.obj");

    // float scale = 100;
    // Vector3 offset(context.width/2.0f, context.height/2.0f, context.depth/4.0f);

    // mesh->Translate(offset, scale, -45.0f);

    // uint32_t materialID = materialBuilder
    //                     .SetBaseColor({0.0f, 0.5f, 0.2f, 1.0f})
    //                     ->SetTransparency(1.0f)
    //                     ->SetRoughness(0.5f)
    //                     ->SetRefractiveIndex(1.45f)
    //                     //->SetSheen(1.0f)
    //                     ->Build();

}

//Main loop

    WindowManager manager(&context);
    PerformanceMonitor monitor;

    ThreadedRendering processor(&context);
    ParallelRendering accelerator(&context);

    manager.BindRenderingService(0, &accelerator);
    manager.BindRenderingService(1, &processor);

    while (manager.ShouldClose()) {
        manager.Update();
        monitor.GatherInformation();
    }

    return EXIT_SUCCESS;
}
