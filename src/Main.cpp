
#include "OpenGLRenderer.h"
#include "MaterialBuilder.h"

#define BUFFER_SIZE 100

int ReadLine(FILE * file, char *buffer){

    int i=0;

    while ( i < BUFFER_SIZE ){

        char symbol;
        fscanf(file, "%c", &symbol);

        if(symbol == '\n' || feof(file))
            break;

        buffer[i] = symbol;
        i++;

    }

    i = std::min(BUFFER_SIZE-1, i);

    buffer[i] = '\0';

    return i;
}

int main(int argc, char* argv[]){

    srand(time(NULL));
    int VSync = true;

    if( argc > 1)
        VSync = atoi(argv[1]);

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
    p.radius = 1000.0f * aspectRatio;
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
            s.position.z = -200.f + (rand() / (float)RAND_MAX) * spacing ;
            s.maxPos = s.position + (Vector3){1, 1, 1} * spacing * aspectRatio;
            s.radius = spacing/2.0f;
            s.normal = Vector3(0.0f, 1.0f, 0.0f);
            s.type = CUBE;

            float isMetallic = (rand() / (float)RAND_MAX)>0.7f;
            float isEmissive = (rand() / (float)RAND_MAX)>0.8f;

            Color color = (Color){(rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX)};


            s.materialID =  materialBuilder
                            .SetBaseColor( color )
                            ->SetDiffuseColor( color )
                            ->SetSpecularColor((Color){(rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX)})
                            ->SetEmissionColor((Color){(rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX)} * isEmissive)
                            ->SetRefractiveIndex((rand() / (float)RAND_MAX))
                            ->SetTransparency(0.0f)
                            ->SetDiffusion((rand() / (float)RAND_MAX))
                            ->SetSmoothness((rand() / (float)RAND_MAX) * isMetallic)
                            ->SetEmission((rand() / (float)RAND_MAX) * isEmissive)
                            ->Build();

            context.objects.emplace_back(s);
        }
    }

}
*/

/*
{
    uint32_t materialID = materialBuilder
                      .SetEmissionColor(255, 255, 255)
                      ->SetEmission(1.0f)
                      ->Build();    


    FILE * input = fopen("resources/mesh.obj", "rb");

    if ( input == NULL ){
        fprintf(stderr, "File can't be opened\n");
        return EXIT_FAILURE;
    }

    std::vector<int> indices;

    char buffer[BUFFER_SIZE] = {0};

    int a, b, c;
    float x, y, z;

    while ( true ){

        int character_count = ReadLine(input, buffer);

        if( character_count == 0 || feof(input))
            break;

        char *token = strtok(buffer, " ");
        char * numA = strtok(NULL, " ");
        char * numB = strtok(NULL, " ");
        char * numC = strtok(NULL, " ");

        switch (buffer[0]){

        case 'v':

            x = atof(numA);
            y = atof(numB);
            z = atof(numC);

            if(buffer[1] == '\0'){
                context.vertices.emplace_back((Vector3){x, y, z});
            }

            break;

        case 'f':
            
            a = atoi(numA);
            b = atoi(numB);
            c = atoi(numC);

            indices.emplace_back(a);
            indices.emplace_back(b);
            indices.emplace_back(c);
            break;

        default:
            break;
        }

    }

    fclose(input);

    for( int i = 0; i< indices.size(); i+=3){

        Object p;
        p.type = TRIANGLE;

        Vector3 a = context.vertices[ indices[i]-1 ];
        Vector3 b = context.vertices[ indices[i+1]-1 ];
        Vector3 c = context.vertices[ indices[i+2]-1 ];

        p.position = a;

        Vector3 u = b - a;
        Vector3 v = c - a;

        Vector3 normal = Vector3::CrossProduct(v, u).Normalize();

        p.indicesID = (Vector3){indices[i]-1, indices[i+1]-1, indices[i+2]-1};

        p.materialID = materialID;

        context.objects.emplace_back(p);
        context.normals.emplace_back(normal);
    }

}
*/

fprintf(stdout, "Object count : %d\n", context.objects.size());
OpenGLRenderer renderer(&context, VSync);

//Main loop

    while (!renderer.ShouldClose()) {
        renderer.Update();
    }

    return EXIT_SUCCESS;
}
