#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <thread>

#include "OpenGLRenderer.h"

int main(){

    srand(time(NULL));

//Context setup

    RenderingContext context;

    context.width = 1000;
    context.height = 1000;
    context.depth = 480;

    float aspectRatio = context.width/(float)context.height;

    context.sun.position = Vector3(context.width/2.0f, 1.1f * context.height, -context.depth);
    context.sun.radius = 300.0f * aspectRatio ;

    context.sun.material = {0};
    context.sun.material.emission = {1.0f, 1.0f , 1.0f, 1.0f};
    context.sun.material.emmissionScale = 1.0f;

    context.spheres.emplace_back(context.sun);

    context.camera.position = Vector3(context.width/2.0f, context.height/2.0f, -700.0f);
    context.camera.aspectRatio = aspectRatio;
{
    //Plane

    Sphere p={0};

    p.position = Vector3(context.width/2.0f, -350.0f, 400.0f);
    p.radius = 1000.0f * aspectRatio;

    p.material.baseColor = {0.5f, 0.0f, 1.0f, 1.0f};
    p.material.specular = {1.0f, 1.0f, 1.0f, 1.0f};
    p.material.emmissionScale = 0;

    context.spheres.emplace_back(p);
    p={0};

    //Spheres

    //Green
    p.position = Vector3(context.width/4.0f+50.0f, context.height/2.0f, -200.0f);
    p.radius = 50.0f * aspectRatio;

    p.material.baseColor = {0.0f, 1.0f, 0.0f, 1.0f};
    p.material.specular = {1.0f, 1.0f, 1.0f, 1.0f};
    p.material.diffusionScale = 0.2f;

    context.spheres.emplace_back(p);
    p={0};

    //Mirror
    p.position = Vector3(context.width/2.0f+50.0f, context.height/2.0f-50.0f, -300.0f);
    p.radius = 75.0f * aspectRatio;

    p.material.baseColor = {0.5f, 0.5f, 0.5f, 1.0f};
    p.material.smoothness = 1.0f;
    p.material.diffusionScale = 0.8f;

    context.spheres.emplace_back(p);
    p={0};

    //Blue
    p.position = Vector3(context.width/2.0f + context.width/4.0f, context.height/2.0f, -200.0f);
    p.radius = 100.0f * aspectRatio;

    p.material.baseColor = {0.0f, 0.0f, 1.0f, 0.1f};

    context.spheres.emplace_back(p);
    p={0};

    //Light
    p.position = Vector3(context.width/2.0f-70.0f, context.height/2.0f+60.0f, -150.0f);
    p.radius = 80.0f * aspectRatio;

    p.material.emission = {1.0f, 0.0f, 0.0f, 1.0f};
    p.material.emmissionScale = 0.7f;

    context.spheres.emplace_back(p);
    p={0};

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
            Sphere s;
            s.position.x = startX + i * spacing;
            s.position.y = startY + j * spacing;
            s.position.z = -200.f;
            s.radius = 32.0f;

            float isMetallic = (rand() / (float)RAND_MAX)>0.7f;
            float isEmissive = (rand() / (float)RAND_MAX)>0.6f;

            s.material.baseColor = {(rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX)};
            s.material.diffuse = {(rand() / (float)RAND_MAX),(rand() / (float)RAND_MAX),(rand() / (float)RAND_MAX)};
            s.material.specular = {(rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX)};

            s.material.smoothness = (rand() / (float)RAND_MAX) * isMetallic;
            s.material.emmissionScale = (rand() / (float)RAND_MAX) * isMetallic;

            s.material.baseColor = s.material.baseColor * (1-isEmissive);
            s.material.emission = (struct Color){(rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX)} * isEmissive;


            context.spheres.emplace_back(s);
            s=(struct Sphere){0};
        }
    }
}
*/

//Objects setup

    OpenGLRenderer renderer(&context);

    Timer *timer = Timer::GetInstance();

//Main loop

    // float angle = 0.0f;
    // float speed = 2.0f;

    while (!renderer.ShouldClose()) {

        // context.sun.position.x = cos(angle * CL_M_PI * deltaTime) * 200.0f + context.width / 2.0f;
        // context.sun.position.z = sin(angle * CL_M_PI * deltaTime) * 200.0f + context.width / 2.0f;

        // context.spheres[0].position = context.sun.position;
        renderer.Update();


        // angle = (angle<=360.0f)*(angle + speed * deltaTime);
    }

    fprintf(stdout, "\nProgram exited succesfully.\n");

    return 0;
}
