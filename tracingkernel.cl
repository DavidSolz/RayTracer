// Structures

struct Material {
    float4 baseColor;
    float4 diffuse;
    float4 specular;
    float4 emission;
    float smoothness;
    float emmissionScale;
    float diffusionScale;
} ;

struct Ray{
    float3 origin;
    float3 direction;
};

struct Sphere{
    float radius;
    float3 position;
    unsigned int materialID;
};

struct HitInfo{
    float distance;
    float3 point;
    float3 normal;
    unsigned int materialID;
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

// Functions

// PCG_Hash
float Rand(unsigned int * seed){
    unsigned int state = *seed *747796405u + 2891336453u;
    unsigned int word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    *seed = (word>>22u) ^ word;
    return *seed/4294967295.0f;
}

// Uniform distrubution
float UniformRandom(unsigned int * seed){
    float theta = 2 * M_PI * Rand(seed);
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
            info.point = ray->origin + ray->direction * distance;
            float3 objPos = (float3)(objects[i].position.x, objects[i].position.y, objects[i].position.z);
            info.normal = normalize(info.point - objPos);
            info.materialID = objects[i].materialID;
        }

    }
    

    return info;
}


float4 ComputeColor(struct Ray *ray, global const struct Sphere* objects, global const int * objects_count, global const struct Material* materials,  unsigned int *seed) {

    float4 accumulatedColor = 0.0f;
    float4 colorMask = 1.0f;
    float intensity = 1.0f;

    for(int i = 0; i < 10; ++i){
        struct HitInfo info = FindClosestIntersection(objects, objects_count, ray);
        
        if(info.distance == INFINITY){
            accumulatedColor += (float4)(0.6f, 0.7f, 0.9f, 1.0f) * intensity;
            break;
        }

        ray->origin = info.point;

        struct Material material = materials[info.materialID];

        float3 diffusion = normalize(RandomReflection(&info.normal, seed) + info.normal);
        float3 reflection = Reflect(&ray->direction, &info.normal);

        ray->direction = diffusion + (reflection - diffusion) * material.smoothness;

        float4 emmisionComponent = material.emission * material.emmissionScale;
        float4 diffuseComponent = material.diffuse * material.diffusionScale;

        accumulatedColor += (emmisionComponent * colorMask);
        accumulatedColor += (diffuseComponent * colorMask);
        colorMask *= material.baseColor * intensity;
        intensity *= 0.1f;
    }

    return accumulatedColor;
}

float3 CalculatePixelPosition(const int x, const int y, const int width, const int height, global const struct Camera * camera){
    float tanHalfFOV = tan(radians(camera->fov) * 0.5f);
    float cameraX = (2.0 * x / width - 1.0f) * camera->aspectRatio * tanHalfFOV * camera->near;
    float cameraY = (2.0 * y / height - 1.0f) * tanHalfFOV * camera->near;
    
    return camera->position + camera->front * camera->near + camera->right * cameraX + camera->up * cameraY;
}

// Main

void kernel RenderGraphics(
global float4* pixels,
global struct Sphere * objects,
global const int * objects_count,
global struct Material * materials,
global const struct Camera * camera,
global const int *numFrames
){

    int x = get_global_id(0);
    int y = get_global_id(1);

    int width = get_global_size(0);
    int height = get_global_size(1);

    unsigned int index = y * width + x;
    unsigned int seed = (*numFrames<<16) ^ (*numFrames >>13) + index ;

    float3 offset = RandomDirection(&seed);

    float3 pixelPosition = CalculatePixelPosition(x + offset.x - 0.5f, y + offset.y - 0.5f, width, height, camera);

    struct Ray ray;
    ray.origin = camera->position;
    ray.direction = normalize(pixelPosition - ray.origin );

    float4 finalColor = 0;

    // Monte - Carlo path tracing have issue with glaring (dark and light spots on consistent color)

    finalColor = ComputeColor(&ray, objects, objects_count, materials, &seed);

    float scale = 1.0f / (*numFrames+1);

    pixels[index] = pixels[index] + (finalColor - pixels[index]) * scale;
}

