
#include "WindowManager.h"
#include "MaterialBuilder.h"
#include "PerformanceMonitor.h"
#include "MeshReader.h"

int main(int argc, char* argv[]){

// Objects setup

RenderingContext context;

// Argument parsing

    const char* sceneFile = nullptr;

    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];

        if(arg[0] != '-'){
            fprintf(stderr, "Unknown argument: %s\n", arg);
            return EXIT_FAILURE;
        }else if (arg[1] == 'V' && arg[2] == '\0') {
            fprintf(stdout, "VSync enabled.\n");
            context.vSync = true;
        } else if (arg[1] == 'L' && arg[2] == '\0') {
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                sceneFile = argv[i + 1];
                i++;
            } else {
                fprintf(stderr, "Error: -L flag requires a scene file path\n");
                return EXIT_FAILURE;
            }
        } else if (arg[1] == 'S' && arg[2] == '\0') {
            fprintf(stdout, "Memory sharing enabled.\n");
            context.memorySharing = true;
        } else {
            fprintf(stderr, "Unknown argument: %s\n", arg);
            return EXIT_FAILURE;
        }
    }

// Context setup

    srand(time(NULL));

    context.width = 1000;
    context.height = 640;
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
                    .SetBaseColor({1.0f, 1.0f, 1.0f, 1.0f})
                    ->SetEmission(1.0f)
                    ->Build();

    context.objects.emplace_back(p);


    const int rows = 10;
    const int cols = 10;
    const float spacing = 100.0f;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            Object s;

            //s.position = Vector3(context.width/2.0f, context.height/2.0f, context.depth/4.0f);
            s.position.x = i * spacing;
            s.position.z = context.height/2.0f;
            s.position.y = j * spacing;
            s.radius = spacing/2.0f;
            s.type = SPHERE;

            float t = (i+j)/(float)(rows);

            float isEmissive = (rand() / (float)RAND_MAX)>0.4f;
            float isMetallic = (rand() / (float)RAND_MAX)>0.8f;
            float isGlass = (rand() / (float)RAND_MAX)>0.8f;

            // const Color colorA = (Color){1.0f, 0.0f, 0.0f, 1.0f};
            // const Color colorB = (Color){0.0f, 0.0f, 1.0f, 1.0f};
            // Color color = Color::Lerp(colorA, colorB, t);

            Color color = (Color){(rand() / (float)RAND_MAX),(rand() / (float)RAND_MAX),(rand() / (float)RAND_MAX), 1.0f};

            s.materialID =  materialBuilder
                            .SetBaseColor(color)
                            ->SetDiffusionColor(color)
                            ->SetSpecularColor(color)
                            ->SetRefractiveIndex((rand() / (float)RAND_MAX) + 1.0f)
                            ->SetSpecularIntensity((rand() / (float)RAND_MAX))
                            ->SetSmoothness((rand() / (float)RAND_MAX) * isMetallic  * (1.0f-isGlass))
                            ->SetEmission(((rand() - RAND_MAX)/ (float)RAND_MAX) * isEmissive)
                            ->SetRoughness((rand() / (float)RAND_MAX))
                            ->SetTransparency((rand() / (float)RAND_MAX) * isGlass * (1.0f - isMetallic))
                            ->Build();

            context.objects.emplace_back(s);
        }
    }

}


/*
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
                    .SetBaseColor({1.0f, 1.0f, 1.0f, 1.0f})
                    ->SetEmission(1.0f)
                    ->Build();

    context.objects.emplace_back(p);

// SUN

    p.position = Vector3(context.width/2.0f, context.height/2.0f, context.depth/4.0f);
    p.position = p.position + move * i * 220.0f;
    p.radius = 100.0f;
    p.type = SPHERE;

    p.materialID = materialBuilder
                    .SetBaseColor((Color){1.0f, 0.0f, 0.0f, 1.0f})
                    ->SetSmoothness(1.0f - i * (1.0f/5))
                    ->SetRoughness( i * (1.0f/5))
                    ->SetSheen(0.5f)
                    ->Build();

    context.objects.emplace_back(p);


    

    p.position = Vector3(3*context.width/2.0f, context.height/2.0f, context.depth);
    p.radius = 100.0f;
    p.type = SPHERE;

    p.materialID = materialBuilder
                    .SetBaseColor((Color){0.0f, 0.0f, 1.0f, 1.0f})
                    ->SetTransparency(1.0f)
                    ->SetRoughness(0.5f)
                    ->Build();

    context.objects.emplace_back(p);

    // MeshReader reader;

    // Mesh mesh = reader.LoadObject("resources/mesh.obj");

    // Object temp;
    // temp.type = TRIANGLE;
    // temp.materialID = materialBuilder
    //                     .SetBaseColor({1.0f, 1.0f, 1.0f, 1.0f})
    //                     ->SetTransparency(1.0f)
    //                     //->SetSmoothness(1.0f)
    //                     ->Build();

    // float scale = 100;
    // Vector3 offset(context.width/2.0f, context.height/2.0f, context.depth/4.0f);

    // mesh.Translate(offset, scale);

    // for(uint32_t i=0; i < mesh.numIndices; ++i){

    //     uint32_t t = 3*i;

    //     temp.indicesID.x = mesh.indices[t];
    //     temp.indicesID.y = mesh.indices[t+1];
    //     temp.indicesID.z = mesh.indices[t+2];
    //     temp.normal = mesh.normals[i];

    //     context.objects.emplace_back(temp);

    // }

    // context.mesh = mesh;
}
*/

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
