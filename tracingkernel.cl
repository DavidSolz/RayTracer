//Structures

struct Color{
    float R, G, B, A;
};

struct Material {
    struct Color baseColor;
    struct Color diffuse;
    struct Color specular;
    struct Color emission;
    float smoothness;
    float emmissionScale;
    float specularScale;
    float diffusionScale;
    float transparencyScale;
};

struct Vector3{
    float x, y, z;
};

struct Ray{
    struct Vector3 origin;
    struct Vector3 direction;
};

struct Sphere{
    float radius;
    struct Vector3 position;
    struct Material material;
};

struct HitInfo{
    float distance;
    struct Vector3 point;
    struct Vector3 normal;
    struct Material material;
};

struct Camera{

    struct Vector3 front;
    struct Vector3 up;
    struct Vector3 right;

    struct Vector3 position;

    float movementSpeed;
    float rotationAngle;
    float aspectRatio;
    float near;
    float far;
    float fov;
};

//Functions

struct Vector3 Sub(const struct Vector3 vectorA, const struct Vector3 vectorB) {
    return (struct Vector3){
        vectorA.x - vectorB.x,
        vectorA.y - vectorB.y,
        vectorA.z - vectorB.z
    };
}

struct Vector3 Add(const struct Vector3 vectorA, const struct Vector3 vectorB) {
    return (struct Vector3){
        vectorA.x + vectorB.x,
        vectorA.y + vectorB.y,
        vectorA.z + vectorB.z
    };
}

struct Vector3 Mult(const struct Vector3 vectorA, const float scalar){
    return (struct Vector3){
        vectorA.x * scalar,
        vectorA.y * scalar,
        vectorA.z * scalar
    };
}

float Magnitude(const struct Vector3 a){
    return sqrt(a.x*a.x+a.y*a.y+a.z*a.z);
}

float DotProduct(const struct Vector3 a, const struct Vector3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

struct Vector3 CrossProduct(const struct Vector3 a, const struct Vector3 b){
    return (struct Vector3){
        a.y*b.z-b.y*a.z,
        a.x*b.z-b.x*a.z,
        a.x*b.y-b.x*a.y
    };
}

struct Vector3 Normalize(const struct Vector3 a){
    float magnitude = Magnitude(a);
    return (struct Vector3){
        a.x/magnitude,
        a.y/magnitude,
        a.z/magnitude
    };
}

//WangHash
float Rand(unsigned int * seed){
    *seed = (*seed ^ 61) ^ (*seed >>16); 
    *seed*=9;
    *seed= *seed ^ (*seed>>4);
    *seed *= 0x27d4eb2d;
    *seed = *seed ^ (*seed>>15);
    return *seed/4294967295.0f;
}

//Uniform distrubution
float UniformRandom(unsigned int * seed){
    float theta = 2 * 3.1415926f * Rand(seed);
    float rho = sqrt(-2 * log(Rand(seed)));
    return rho * cos(theta);
}

struct Vector3 RandomDirection(unsigned int *seed){
    return Normalize((struct Vector3){
        UniformRandom(seed),
        UniformRandom(seed),
        UniformRandom(seed)
    });
}

struct Vector3 RandomReflection(const struct Vector3 normal, unsigned int *seed){
    struct Vector3 direction = RandomDirection(seed);
    return Mult(direction, sign(DotProduct(normal, direction)));
}

struct Color AddColors(struct Color colorA, struct Color colorB){
    return (struct Color){
        colorA.R + colorB.R,
        colorA.G + colorB.G,
        colorA.B + colorB.B,
        colorA.A + colorB.A
    };
}

struct Color LerpColor(struct Color colorA, struct Color colorB, float t){
    t = fmax(0.0f, fmin(t, 1.0f));
    return (struct Color){
        colorA.R + t*(colorB.R - colorA.R),
        colorA.G + t*(colorB.G - colorA.G),
        colorA.B + t*(colorB.B - colorA.B),
        fmax(colorA.A, colorB.A)
    };
}

struct Color MixColors(struct Color colorA, struct Color colorB) {
    return (struct Color){
        colorA.R * colorB.R,
        colorA.G * colorB.G,
        colorA.B * colorB.B,
        colorA.A * colorB.A
    };
}

struct Color ToneColor(struct Color color, float factor) {
    factor = fmax(0.0f, fmin(factor, 1.0f));
    return (struct Color){
        color.R * factor,
        color.G * factor,
        color.B * factor,
        color.A * factor
    };
}

float Intersect(const struct Ray *ray, global const struct Sphere *sphere) {
    struct Vector3 originToSphereCenter = Sub(ray->origin, sphere->position);

    float b = DotProduct(originToSphereCenter, ray->direction);
    float c = DotProduct(originToSphereCenter, originToSphereCenter) - sphere->radius * sphere->radius;
    float delta = b * b - c;

    float sqrtDelta =  sqrt(delta);
    float t1 = (-b - sqrtDelta);
    float t2 = (-b + sqrtDelta);

    return fmin(t1, t2);
}

struct Vector3 LerpPoint(struct Vector3 begin, struct Vector3 end, float t){
    t = fmax(0.0f, fmin(t, 1.0f));
    return Add(begin, Mult(Sub(end,begin), t));
}

struct Vector3 Reflect(struct Vector3 incident, struct Vector3 normal) {
    return Sub(incident, Mult(normal, (2.0f * DotProduct(incident, normal))));
}


struct HitInfo FindClosestIntersection(global const struct Sphere* objects, global const int * objects_count, const struct Ray * ray){
    struct HitInfo info = {0};
    info.distance = INFINITY;
    
    for (int i = 0; i < *objects_count; ++i) {
        float distance = Intersect(ray, objects + i);

        if((distance < info.distance) && (distance>0.0f)){
            info.distance = distance ;
            info.point = Add(ray->origin, Mult(ray->direction, distance * 1.05f));
            info.normal = Normalize(Sub(info.point, objects[i].position));
            info.material = objects[i].material;
        }

    }
    

    return info;
}


struct Color ComputeColor(struct Ray *ray, global const struct Sphere* objects, global const int * objects_count, unsigned int *seed) {

    struct Color accumulatedColor = {0};
    struct Color tempColor = {1.0f, 1.0f, 1.0f, 1.0f};

    for(int i = 0; i < 30; ++i){
        struct HitInfo info = FindClosestIntersection(objects, objects_count, ray);
        
        if(info.distance == INFINITY)
            break;

        ray->origin = info.point;

        struct Material * material = &info.material;

        struct Vector3 diffusionDir = RandomReflection(info.normal, seed);
        struct Vector3 specularDir = Reflect(ray->direction, info.normal);
        
        ray->direction = LerpPoint(diffusionDir, specularDir, material->smoothness);

        struct Color emmisionComponent = ToneColor(material->emission, material->emmissionScale );

        float lightStrength = DotProduct(info.normal, ray->direction);
        accumulatedColor = AddColors(accumulatedColor, MixColors(emmisionComponent, tempColor));
        tempColor = MixColors(tempColor, ToneColor(material->baseColor, lightStrength));

    }

    return accumulatedColor;
}

struct Vector3 CalculatePixelPosition(const int x, const int y, const int width, const int height, global const struct Camera * camera){
    float tanHalfFOV = tan(radians(camera->fov / 2.0));
    float cameraX = (2.0 * x / width - 1.0f) * camera->aspectRatio * tanHalfFOV * camera->near;
    float cameraY = (2.0 * y / height - 1.0f) * tanHalfFOV * camera->near;
    
    return (struct Vector3){
        camera->position.x + camera->front.x * camera->near + camera->right.x * cameraX + camera->up.x * cameraY,
        camera->position.y + camera->front.y * camera->near + camera->right.y * cameraX + camera->up.y * cameraY,
        camera->position.z + camera->front.z * camera->near + camera->right.z * cameraX + camera->up.z * cameraY
    };
}

//Main

void kernel RenderGraphics(global struct Color* pixels,
global struct Sphere* objects,
global const int * objects_count,
global const struct Camera * camera,
global const int *numFrames){

    int x = get_global_id(0);
    int y = get_global_id(1);

    int width = get_global_size(0);
    int height = get_global_size(1);

    unsigned int index = y * width + x;
    unsigned int seed = *numFrames * 92037129381 + index ;

    struct Vector3 offset = RandomDirection(&seed);

    struct Vector3 pixelPosition = CalculatePixelPosition(x + offset.x - 0.5f, y + offset.y - 0.5f, width, height, camera);

    
    struct Ray ray;
    ray.origin = camera->position; 
    ray.direction = Normalize(Sub(pixelPosition, ray.origin ));

    struct Color finalColor = {0};

    for(int i=0; i<10; i++){
        finalColor = AddColors(finalColor, ComputeColor(&ray, objects, objects_count, &seed));
    }

    float scale = 1.0f / (*numFrames+1);

    pixels[index] = LerpColor(pixels[index], finalColor, scale);
}

