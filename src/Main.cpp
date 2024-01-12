
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
                    .SetBaseColor({0.0f, 0.0f, 1.0f, 1.0f})
                    ->SetDiffusion(0.4f)
                    ->Build();

    context.objects.emplace_back(p);

// RED CUBE

    p.position = Vector3(context.width/3.0f, context.height/4.0f+20.0f, context.depth/2.0f);
    p.maxPos = p.position + Vector3(200, 200, 200);
    p.type = CUBE;

    p.materialID = materialBuilder
                    .SetBaseColor({1.0f, 0.0f, 0.0f, 1.0f})
                    ->SetDiffusion(0.2f)
                    ->Build();

    context.objects.emplace_back(p);

// GREEN CUBE

    p.position = Vector3(context.width/5.0f, context.height/4.0f+20.0f, 3*context.depth/4.0f);
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
                    ->SetSmoothness(1.52f)
                    ->Build();

    context.objects.emplace_back(p);

// GLASS

    p.position = Vector3(context.width/4.0f, context.height/4.0f + 50.0f, 0.0f);
    p.type = SPHERE;
    p.radius = 50.0f;

    p.materialID = materialBuilder
                    .SetBaseColor({0.7f, 0.7f, 0.7f, 1.0f})
                    ->SetRefractiveIndex(1.52f)
                    ->SetTransparency(1.0f)
                    ->Build();

    context.objects.emplace_back(p);


// PLANE

    p.position = Vector3(0, 0, -context.depth/4.0f);
    p.maxPos = p.position + Vector3(context.width, context.height/4.0f, 2*context.depth);
    p.type = CUBE;

    p.materialID = materialBuilder
                    .SetBaseColor({1.0f, 1.0f, 1.0f, 1.0f})
                    ->SetDiffusion(0.1f)
                    ->Build();

    context.objects.emplace_back(p);

// WALL
    p.position = Vector3(0, 0, -context.depth/4.0f + 2*context.depth);
    p.maxPos = p.position + Vector3(context.width, context.height, context.depth/4.0f);
    p.type = CUBE;

    p.materialID = materialBuilder
                    .SetEmissionColor({1.0f, 1.0f, 1.0f, 1.0f})
                    ->SetEmission(0.7f)
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
            s.position.z = -200.f + (rand() / (float)RAND_MAX) * spacing * aspectRatio;
            s.maxPos = s.position + (Vector3){spacing, spacing, spacing} * aspectRatio;
            s.type = CUBE;

            float isMetallic = (rand() / (float)RAND_MAX)>0.7f;
            float isEmissive = (rand() / (float)RAND_MAX)>0.8f;

            Color color = (Color){(rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX)} * (1- isEmissive);


            s.materialID =  materialBuilder
                            .SetBaseColor( color )
                            ->SetDiffuseColor( color )
                            ->SetSpecularColor({(rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX)})
                            ->SetEmissionColor((Color){(rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX)} * isEmissive)
                            ->SetRefractiveIndex((rand() / (float)RAND_MAX))
                            ->SetDiffusion((rand() / (float)RAND_MAX))
                            ->SetSmoothness((rand() / (float)RAND_MAX) * isMetallic)
                            ->SetEmission((rand() / (float)RAND_MAX) * isEmissive)
                            ->Build();

            context.objects.emplace_back(s);
        }
    }
}
*/


OpenGLRenderer renderer(&context);

//Main loop

    while (!renderer.ShouldClose()) {
        renderer.Update();
    }

    return EXIT_SUCCESS;
}
