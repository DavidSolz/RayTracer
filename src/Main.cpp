
#include "WindowManager.h"
#include "MaterialBuilder.h"
#include "PerformanceMonitor.h"
#include "MeshReader.h"

int main(int argc, char* argv[]){

    int VSync = true;

    if( argc > 1)
        VSync = atoi(argv[1]);

// Objects setup

    RenderingContext context;

// Context setup

    context.width = 1000;
    context.height = 1000;
    context.depth = 480;

    context.camera.position = Vector3(context.width/2.0f, context.height/2.0f, -900.0f);
    context.camera.aspectRatio = context.width/(float)context.height;

    context.loggingService.BindOutput("RayTracer_log.txt");

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


/*
{
    const int rows = 10;
    const int cols = 10;
    const float spacing = 60.0f;
    const float factor = 0.001f;

    float startX = (context.width - (cols - 1) * spacing) / 2.0f ;
    float startY = (context.height - (rows - 1) * spacing) / 2.0f ;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            Object s;

            s.position.x = startX + i * spacing;
            s.position.y = startY + j * spacing;
            s.position.z = -200.0f - factor * ( pow(s.position.x - context.width/2, 2) + pow(s.position.y - context.height/2, 2));
            s.maxPos = s.position + (Vector3){1, 1, 1} * spacing;
            s.radius = spacing/2.0f;
            s.normal = Vector3(0.0f, 1.0f, 0.0f);
            s.type = CUBE;

            float isEmissive = (rand() / (float)RAND_MAX)>0.8f;
            float isMetallic = (rand() / (float)RAND_MAX)>0.8f;
            float isGlass = (rand() / (float)RAND_MAX)>0.5f;

            Color color = (Color){(rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX)};


            s.materialID =  materialBuilder
                            .SetBaseColor(color)
                            ->SetSpecularColor(color)
                            ->SetEmissionColor((Color){1.0f, 1.0f, 1.0f, 1.0f} * isEmissive * (1-isGlass))
                            ->SetRefractiveIndex((rand() / (float)RAND_MAX) + 1.0f)
                            ->SetDiffusion((rand() / (float)RAND_MAX))
                            ->SetSmoothness((rand() / (float)RAND_MAX) * isMetallic  * (1-isGlass))
                            ->SetEmission(((rand() - RAND_MAX)/ (float)RAND_MAX) * isEmissive * (1 - isMetallic))
                            ->SetRoughness( (rand() / (float)RAND_MAX) * (1-isMetallic) * (1-isGlass))
                            ->SetTransparency(1.0f  * isGlass)
                            ->Build();

            context.objects.emplace_back(s);
        }
    }

}
*/


{
// DISK

    Object p;

    p.position = Vector3(context.width/2.0f, context.height/4.0f, context.depth/4.0f);
    p.normal = Vector3(0.0f, 1.0f ,0.0f);
    p.type = PLANE;

    p.materialID = materialBuilder
                    .SetBaseColor({0.5f, 0.5f, 0.5f, 1.0f})
                    ->Build();

    context.objects.emplace_back(p);

// Light
    p.position = Vector3(context.width/2.0f, 2*context.height, context.depth/4.0f);
    p.normal = Vector3(0.0f, -1.0f ,0.0f);
    p.radius = 300.0f;
    p.type = DISK;

    p.materialID = materialBuilder
                    .SetEmissionColor({1.0f, 1.0f, 1.0f, 1.0f})
                    ->SetEmission(10.0f)
                    ->Build();

    context.objects.emplace_back(p);

// SUN

    p.position = Vector3(context.width/2.0f, context.height/2.0f, context.depth);
    p.radius = 100.0f;
    p.type = SPHERE;

    p.materialID = materialBuilder
                    .SetEmissionColor((Color){1.0f, 0.0f, 0.0f, 1.0f})
                    ->SetEmission(100.0f)
                    ->Build();

    context.objects.emplace_back(p);

    p.position = Vector3(3*context.width/2.0f, context.height/2.0f, context.depth);
    p.radius = 100.0f;
    p.type = SPHERE;

    p.materialID = materialBuilder
                    .SetEmissionColor((Color){0.0f, 0.0f, 1.0f, 1.0f})
                    ->SetEmission(100.0f)
                    ->Build();

    context.objects.emplace_back(p);


    MeshReader reader;

    Mesh mesh = reader.LoadObject("resources/mesh.obj");

    Object temp;
    temp.type = TRIANGLE;
    temp.materialID = materialBuilder
                        .SetBaseColor({1.0f, 1.0f, 1.0f, 1.0f})
                        ->SetRefractiveIndex(2.25f)
                        ->SetTransparency(1.0f)
                        ->SetRoughness(1.0f)
                        ->Build();


    float scale = 100;
    Vector3 offset(context.width/2.0f, context.height/2.0f, context.depth/4.0f);

    mesh.Translate(offset, scale);

    for(uint32_t i=0; i < mesh.numIndices; ++i){

        uint32_t t = 3*i;

        temp.indicesID.x = mesh.indices[t];
        temp.indicesID.y = mesh.indices[t+1];
        temp.indicesID.z = mesh.indices[t+2];
        temp.normal = mesh.normals[i];

        context.objects.emplace_back(temp);

    }

    context.mesh = mesh;
}


//Main loop

    
    ThreadedRendering cpuRender;
    cpuRender.BindContext(&context);
    
    ParallelRendering gpuRender;
    gpuRender.BindContext(&context);

    std::vector<IFrameRender*> services;

    services.emplace_back(&cpuRender);
    services.emplace_back(&gpuRender);

    WindowManager manager(&context, VSync);
    PerformanceMonitor monitor;

   // manager.BindRenderingServices(services.data(), services.size());
    manager.SetDefaultRendering(1);

    while (manager.ShouldClose()) {
        manager.Update();
        monitor.GatherInformation();
    }

    return EXIT_SUCCESS;
}
