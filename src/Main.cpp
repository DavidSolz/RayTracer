
#include "OpenGLRenderer.h"
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
    PerformanceMonitor monitor;

/*
{

    Object p;

// BLUE CUBE

    p.position = Vector3(context.width/2.0f, context.height/4.0f, context.depth);
    p.maxPos = p.position + Vector3(300, 400, 300);
    p.type = CUBE;

    p.materialID = materialBuilder
                    .SetBaseColor({0.0f, 0.0f, 1.0f, 1.0f})
                    ->SetDiffusion(0.4f)
                    ->Build();

    context.objects.emplace_back(p);

// RED CUBE

    p.position = Vector3(context.width/3.0f, context.height/4.0f, context.depth/2.0f);
    p.maxPos = p.position + Vector3(200, 200, 200);
    p.type = CUBE;

    p.materialID = materialBuilder
                    .SetBaseColor({1.0f, 0.0f, 0.0f, 1.0f})
                    ->SetDiffusion(0.2f)
                    ->Build();

    context.objects.emplace_back(p);

// GREEN CUBE

    p.position = Vector3(context.width/5.0f, context.height/4.0f, 3*context.depth/4.0f);
    p.maxPos = p.position + Vector3(100, 100, 100);
    p.type = CUBE;

    p.materialID = materialBuilder
                    .SetBaseColor({0.0f, 1.0f, 0.0f, 1.0f})
                    ->SetDiffusion(0.1f)
                    ->Build();

    context.objects.emplace_back(p);

// MIRROR

    p.position = Vector3(context.width/2.0f, context.height/4.0f + 50.0f, 0.0f);
    p.type = SPHERE;
    p.radius = 50.0f;

    p.materialID = materialBuilder
                    .SetBaseColor({0.5f, 0.5f, 0.5f, 1.0f})
                    ->SetDiffusion(1.0f)
                    ->SetSmoothness(1.0f)
                    ->Build();

    context.objects.emplace_back(p);

    p.position = Vector3(context.width/2.0f + 120.0f, context.height/4.0f + 50.0f, 0.0f);
    p.type = SPHERE;
    p.radius = 50.0f;

    p.materialID = materialBuilder
                    .SetBaseColor({0.5f, 0.5f, 0.5f, 1.0f})
                    ->SetDiffusion(1.0f)
                    ->SetSmoothness(0.5f)
                    ->Build();

    context.objects.emplace_back(p);

    p.position = Vector3(context.width/2.0f + 240.0f, context.height/4.0f + 50.0f, 0.0f);
    p.type = SPHERE;
    p.radius = 50.0f;

    p.materialID = materialBuilder
                    .SetBaseColor({0.5f, 0.5f, 0.5f, 1.0f})
                    ->SetDiffusion(1.0f)
                    ->SetSmoothness(0.0f)
                    ->Build();

    context.objects.emplace_back(p);

// RED LIGHT

    p.position = Vector3(context.width/4.0f, context.height/4.0f + 50.0f, 0.0f);
    p.type = SPHERE;
    p.radius = 50.0f;

    p.materialID = materialBuilder
                    .SetEmissionColor({1.0f, 0.0f, 0.0f, 1.0f})
                    ->SetEmission(1.0f)
                    ->Build();

    context.objects.emplace_back(p);


// PLANE

    p.position = Vector3(context.width/2.0f, context.height/4.0f, context.depth/4.0f);
    p.normal = Vector3(0.0f, 1.0f ,0.0f);
    p.radius = 1000.0f;
    p.type = DISK;

    p.materialID = materialBuilder
                    .SetBaseColor(255, 126, 13)
                    ->SetEmissionColor(36, 13, 42)
                    ->SetEmission(1.0f)
                    ->Build();

    context.objects.emplace_back(p);

// SUN
    p.position = Vector3(context.width/2.0f, 2*context.height, context.depth/4.0f);
    p.radius = 600.0f;
    p.type = SPHERE;

    p.materialID = materialBuilder
                    .SetEmissionColor({1.0f, 1.0f, 1.0f, 1.0f})
                    ->SetEmission(1.0f)
                    ->Build();

    context.objects.emplace_back(p);

}
*/


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

            Color color = (Color){(rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX)};


            s.materialID =  materialBuilder
                            .SetBaseColor(color)
                            ->SetSpecularColor(color)
                            ->SetEmissionColor((Color){1.0f, 1.0f, 1.0f, 1.0f} * isEmissive)
                            ->SetRefractiveIndex((rand() / (float)RAND_MAX) + 1e-6f)
                            ->SetDiffusion((rand() / (float)RAND_MAX))
                            ->SetSmoothness((rand() / (float)RAND_MAX) * (1.0f - isEmissive) )
                            ->SetEmission(((rand() - RAND_MAX)/ (float)RAND_MAX) * isEmissive )
                            ->SetRoughness( (1.0f -  (rand() / (float)RAND_MAX) ) * (1.0f - isEmissive))
                            ->SetTransparency((rand() / (float)RAND_MAX))
                            ->Build();

            context.objects.emplace_back(s);
        }
    }

}



// {
// // DISK

//     Object p;

//     p.position = Vector3(context.width/2.0f, context.height/4.0f, context.depth/4.0f);
//     p.normal = Vector3(0.0f, 1.0f ,0.0f);
//     p.radius = 1000.0f;
//     p.type = DISK;

//     p.materialID = materialBuilder
//                     .SetBaseColor({0.5f, 0.5f, 0.5f, 1.0f})
//                     ->SetDiffusion(0.1f)
//                     ->SetSmoothness(0.1f)
//                     ->Build();

//     context.objects.emplace_back(p);

// // SUN

//     p.position = Vector3(context.width/2.0f, 2*context.height, context.depth/4.0f);
//     p.radius = 600.0f;
//     p.type = SPHERE;

//     p.materialID = materialBuilder
//                     .SetEmissionColor((Color){1.0f, 1.0f, 1.0f, 1.0f})
//                     ->SetEmission(1.0f)
//                     ->Build();

//     context.objects.emplace_back(p);


//     MeshReader reader;

//     Mesh mesh = reader.LoadObject("resources/mesh.obj");

//     Object temp;
//     temp.type = TRIANGLE;
//     temp.materialID = materialBuilder
//                         .SetBaseColor({1.0f, 0.843f, 0.0f, 1.0f})
//                         ->SetSpecularColor({1.0f, 0.843f, 0.0f, 1.0f})
//                         ->SetDiffusion(0.2f)
//                         ->SetRefractiveIndex(1.26f)
//                         ->SetTransparency(0.6f)
//                         ->SetSmoothness(1.0f)
//                         ->SetRoughness(0.0f)
//                         ->Build();

//     float scale = 100;
//     Vector3 offset(context.width/2.0f, context.height/2.0f, context.depth/4.0f);

//     mesh.Translate(offset, scale);

//     for(uint32_t i=0; i < mesh.numIndices; ++i){

//         uint32_t t = 3*i;

//         temp.indicesID.x = mesh.indices[t];
//         temp.indicesID.y = mesh.indices[t+1];
//         temp.indicesID.z = mesh.indices[t+2];
//         temp.normal = mesh.normals[i];

//         context.objects.emplace_back(temp);

//     }

//     context.mesh = mesh;
// }


//Main loop

    OpenGLRenderer renderer(&context, VSync);

    while (!renderer.ShouldClose()) {
        renderer.Update();
        monitor.GatherInformation();
    }

    return EXIT_SUCCESS;
}
