//Structures

struct Material {
    float4 baseColor;
    float4 diffuse;
    float4 specular;
    float4 emission;
    float smoothness;
    float emmissionScale;
    float specularScale;
    float diffusionScale;
    float transparencyScale;
};

struct Ray{
    float3 origin;
    float3 direction;
};

struct Sphere{
    float radius;
    float3 position;
    struct Material material;
};

struct HitInfo{
    float distance;
    float3 point;
    float3 normal;
    struct Material material;
};

struct Camera{

    float3 front;
    float3 up;
    float3 right;

    float3 position;

    float movementSpeed;
    float rotationAngle;
    float aspectRatio;
    float near;
    float far;
    float fov;
};

//Functions

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

float3 RandomDirection(unsigned int *seed){
    return normalize((float3)(
        UniformRandom(seed),
        UniformRandom(seed),
        UniformRandom(seed)
    ));
}

float3 RandomReflection(float3 * normal, unsigned int *seed){
    float3 direction = RandomDirection(seed);
    return direction * sign(dot(*normal, direction));
}

float Intersect(const struct Ray *ray, global const struct Sphere *sphere) {
    float3 spherePos = (float3)(sphere->position.x, sphere->position.y, sphere->position.z);
    float3 originToSphereCenter = ray->origin - spherePos;

    float b = dot(originToSphereCenter, ray->direction);
    float c = dot(originToSphereCenter, originToSphereCenter) - sphere->radius * sphere->radius;
    float delta = b * b - c;

    float sqrtDelta =  sqrt(delta);
    float t1 = (-b - sqrtDelta);
    float t2 = (-b + sqrtDelta);

    return fmin(t1, t2);
}

float3 LerpPoint(const float3 * begin, const float3 * end, float t){
    t = fmax(0.0f, fmin(t, 1.0f));
    return *begin + (*end - *begin) * t;
}

float3 Reflect(const float3 * incident, const float3 * normal) {
    return *incident - *normal * 2.0f * dot(*incident, *normal);
}


struct HitInfo FindClosestIntersection(global const struct Sphere* objects, global const int * objects_count, const struct Ray * ray){
    struct HitInfo info = {0};
    info.distance = INFINITY;
    
    for (int i = 0; i < *objects_count; ++i) {
        float distance = Intersect(ray, objects + i);

        if((distance < info.distance) && (distance>0.0f)){
            info.distance = distance ;
            info.point = ray->origin + ray->direction * distance * 1.05f;
            float3 objPos = (float3)(objects[i].position.x, objects[i].position.y, objects[i].position.z);
            info.normal = normalize(info.point - objPos);
            info.material = objects[i].material;
        }

    }
    

    return info;
}


float4 ComputeColor(struct Ray *ray, global const struct Sphere* objects, global const int * objects_count, unsigned int *seed) {

    float4 accumulatedColor = 0;
    float4 tempColor = (float4)(1.0f, 1.0f, 1.0f, 1.0f);

    for(int i = 0; i < 10; ++i){
        struct HitInfo info = FindClosestIntersection(objects, objects_count, ray);
        
        if(info.distance == INFINITY)
            break;
        

        ray->origin = info.point;

        struct Material * material = &info.material;

        float3 diffusionDir = normalize(RandomReflection(&info.normal, seed) + info.normal);
        float3 specularDir = Reflect(&ray->direction, &info.normal);
        
        ray->direction = LerpPoint(&diffusionDir, &specularDir, material->smoothness);

        float4 emmisionComponent = material->emission * material->emmissionScale;
        float4 diffuseComponent = material->diffuse * dot(info.normal, diffusionDir) * material->diffusionScale;


        accumulatedColor += (emmisionComponent * tempColor);
        accumulatedColor += (diffuseComponent * tempColor);
        tempColor *= material->baseColor;
    }

    return accumulatedColor;
}

float3 CalculatePixelPosition(const int x, const int y, const int width, const int height, global const struct Camera * camera){
    float tanHalfFOV = tan(radians(camera->fov / 2.0));
    float cameraX = (2.0 * x / width - 1.0f) * camera->aspectRatio * tanHalfFOV * camera->near;
    float cameraY = (2.0 * y / height - 1.0f) * tanHalfFOV * camera->near;
    
    return camera->position + camera->front * camera->near + camera->right * cameraX + camera->up * cameraY;
}

//Main

void kernel RenderGraphics(global float4* pixels,
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

    float3 offset = RandomDirection(&seed);

    float3 pixelPosition = CalculatePixelPosition(x + offset.x - 0.5f, y + offset.y - 0.5f, width, height, camera);

    struct Ray ray;
    ray.origin = camera->position;
    ray.direction = normalize(pixelPosition - ray.origin );

    float4 finalColor = 0;

    finalColor = ComputeColor(&ray, objects, objects_count, &seed);

    float scale = 1.0f / (*numFrames+1);

    pixels[index] = pixels[index] + (finalColor - pixels[index]) * scale;
}

