#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <thread>

#include "OpenGLRenderer.h"
#include "Timer.h"

int main(){

    srand(time(NULL));

//Context setup

    RenderingContext context;

    context.width = 640;
    context.height = 480;
    context.depth = 480;

    context.sun.position = Vector3(context.width/2.0f, context.height/2.0f, 0);
    context.sun.radius = 10.0f;

    context.cameraPos = Vector3(context.width/2.0f, context.height/2.0f, -context.depth/2.0f);

    const int rows = 5;
    const int cols = 5;
    const float spacing = 40.0f;

    float startX = (context.width - (cols - 1) * spacing) / 2.0f;
    float startY = (context.depth - (rows - 1) * spacing) / 2.0f;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            Sphere s;
            s.position.x = startX + i * spacing;
            s.position.y = startY + j * spacing;
            s.position.z = context.depth / 2.0f;
            s.radius = 20.0f;

            float randomMaterial = (rand() / (float)RAND_MAX);

            if(randomMaterial > 0.75f){
                
                //Glass
                s.material.ambient = {0, 0, 0};
                s.material.diffuse = {0, 0, 0};
                s.material.specular = {255, 255, 255};
                s.material.shininess = 100.0f;
                s.material.diffuseLevel = 0.1f;
                
            }if(randomMaterial > 0.5f){
                //Emissive
                s.material.ambient = {rand()%255, rand()%255, rand()%255};
                s.material.diffuse = {0};
                s.material.specular = {0};
                s.material.shininess = 50.0f;
                s.material.diffuseLevel = 1.0f;
            }if(randomMaterial > 0.25f){
                //Phong material
                s.material.ambient = {rand()%255, rand()%255, rand()%255};
                s.material.diffuse = Color::Max(s.material.ambient, {0,0,0,0});
                s.material.specular = {255, 255, 255, 255};
                s.material.shininess = 50.0f;
                s.material.diffuseLevel = 1.0f;
            }else{
                //Metall
                s.material.ambient = {0};
                s.material.diffuse = {128, 128, 128};
                s.material.specular = {255, 255, 255};
                s.material.shininess = 50.0f;
                s.material.diffuseLevel = 1.0f; 
            }

            context.spheres.emplace_back(s);
        }
    }

    context.sun.material.ambient={255,255,255};
    context.sun.material.diffuse={255,255,255};
    context.sun.material.shininess=100.0f;
    context.sun.material.specular={255,255,255};

//Objects setup

    OpenGLRenderer renderer(&context);

    Timer *timer = Timer::GetInstance();

//Main loop

    float angle = 0.0f;
    float speed = 10.0f;

    while (!renderer.ShouldClose()) {
        float deltaTime = timer->GetDeltaTime();

        context.sun.position.x = cos(angle * CL_M_PI * deltaTime) * 300.0f + context.width / 2.0f;
        context.sun.position.y = sin(angle * CL_M_PI * deltaTime) * 300.0f + context.width / 2.0f;

        timer->TicTac();

        renderer.Update();

        angle = (angle<=360.0f)*(angle + speed * deltaTime);
    }

    fprintf(stdout, "\nProgram exited succesfully.\n");

    return 0;
}
