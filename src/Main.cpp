
#include "OpenGLRenderer.h"
#include "MaterialBuilder.h"

int main(int argc, char* argv[]){

    srand(time(NULL));


// Objects setup

    RenderingContext context;
    MaterialBuilder materialBuilder(&context);

// Context setup

    context.width = 1000;
    context.height = 1000;
    context.depth = 480;

    float aspectRatio = context.width/(float)context.height;

// Setup camera

    context.camera.position = Vector3(context.width/2.0f, context.height/2.0f, -900.0f);
    context.camera.aspectRatio = aspectRatio;


{

    Object p;

// BLUE CUBE

    p.position = Vector3(context.width/2.0f, context.height/4.0f+20.0f, context.depth);
    p.maxPos = p.position + Vector3(300, 400, 300);
    p.type = CUBE;

    p.materialID = materialBuilder
                    .SetEmissionColor({0.0f, 0.0f, 1.0f, 1.0f})
                    ->SetEmission(1.0f)
                    ->Build();

    context.objects.emplace_back(p);

// RED CUBE

    p.position = Vector3(context.width/3.0f, context.height/4.0f+20.0f, context.depth/2.0f);
    p.maxPos = p.position + Vector3(200, 200, 200);
    p.type = CUBE;

    p.materialID = materialBuilder
                    .SetEmissionColor({1.0f, 0.0f, 0.0f, 1.0f})
                    ->SetEmission(1.0f)
                    ->Build();

    context.objects.emplace_back(p);

// GREEN CUBE

    p.position = Vector3(context.width/4.0f, context.height/4.0f+20.0f, 0.0f);
    p.maxPos = p.position + Vector3(100, 100, 100);
    p.type = CUBE;

    p.materialID = materialBuilder
                    .SetEmissionColor({0.0f, 1.0f, 0.0f, 1.0f})
                    ->SetEmission(1.0f)
                    ->Build();

    context.objects.emplace_back(p);

// MIRROR

    p.position = Vector3(context.width/2.0f, context.height/4.0f + 50.0f, 0.0f);
    p.type = SPHERE;
    p.radius = 50.0f;

    p.materialID = materialBuilder
                    .SetBaseColor({0.5f, 0.5f, 0.5f, 1.0f})
                    ->SetSmoothness(1.0f)
                    ->Build();

    context.objects.emplace_back(p);

// PLANE

    p.position = Vector3(0, 0, -context.depth/4.0f);
    p.maxPos = p.position + Vector3(context.width, context.height/4.0f, 2*context.depth);
    p.type = CUBE;

    p.materialID = materialBuilder
                    .SetBaseColor({1.0f, 1.0f, 1.0f, 1.0f})
                    ->SetDiffusion(1.0f)
                    ->SetSmoothness(0.2f)
                    ->Build();

    context.objects.emplace_back(p);

// WALL
    p.position = Vector3(0, 0, -context.depth/4.0f + 2*context.depth);
    p.maxPos = p.position + Vector3(context.width, context.height, context.depth/4.0f);
    p.type = CUBE;

    p.materialID = materialBuilder
                    .SetBaseColor({1.0f, 1.0f, 1.0f, 1.0f})
                    ->SetDiffusion(1.0f)
                    ->SetSmoothness(0.2f)
                    ->Build();

    context.objects.emplace_back(p);

}

/*
{
    const int rows = 10;
    const int cols = 10;
    const float spacing = 60.0f;

    float startX = (context.width - (cols - 1) * spacing) / 2.0f ;
    float startY = (context.height - (rows - 1) * spacing) / 2.0f ;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            Object s;

            s.position.x = startX + i * spacing;
            s.position.y = startY + j * spacing;
            s.position.z = -200.f;
            s.radius = 32.0f*aspectRatio;
            s.type = SPHERE;

            float isMetallic = (rand() / (float)RAND_MAX)>0.7f;
            float isEmissive = (rand() / (float)RAND_MAX)>0.6f;

            s.materialID =  materialBuilder
                            .SetBaseColor( (Color){(rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX)} * (1- isEmissive))
                            ->SetDiffuseColor({(rand() / (float)RAND_MAX),(rand() / (float)RAND_MAX),(rand() / (float)RAND_MAX)})
                            ->SetSpecularColor({(rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX)})
                            ->SetEmissionColor((Color){(rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX)} * isEmissive)
                            ->SetSmoothness((rand() / (float)RAND_MAX) * isMetallic)
                            ->SetEmission((rand() / (float)RAND_MAX) * isMetallic)
                            ->Build();

            context.objects.emplace_back(s);
            s = Object();
        }
    }
}
*/


OpenGLRenderer renderer(&context, false);

//Main loop

    while (!renderer.ShouldClose()) {

        renderer.Update();

    }

    fprintf(stdout, "\nProgram exited succesfully.\n");

    return EXIT_SUCCESS;
}
