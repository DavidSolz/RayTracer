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

enum SpatialType{
    SPHERE,
    PLANE,
    DISK,
    CUBE,
    CYLINDER
};

struct Ray{
    float3 origin;
    float3 direction;
};

struct Object{
    enum SpatialType type;
    float radius;
    float3 position;
    float3 normal;
    float3 maxPos;
    unsigned int materialID;
};

struct Sample{
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
    *seed = *seed *747796405u + 2891336453u;
    unsigned int word = ((*seed >> ((*seed >> 28u) + 4u)) ^ *seed) * 277803737u;
    return ((word>>22u) ^ word)/(float)UINT_MAX;
}

// Uniform distrubution
float UniformRandom(unsigned int * seed){
    float theta = 2 * M_PI * Rand(seed);
    float rho = sqrt(-2 * log(Rand(seed)));
    return rho * cos(theta);
}

float3 RandomDirection(unsigned int *seed){
    float phi = UniformRandom(seed) * 2 * 3.1415926535f;
    float theta = UniformRandom(seed) * 2 * 3.1415926535f;

    float sinTheta = sin(theta);

    return (float3)(
        sinTheta*cos(phi),
        cos(theta),
        sinTheta*sin(phi)
    );
}

float3 RandomReflection(float3 normal, unsigned int *seed){
    float3 direction = RandomDirection(seed);
    return direction * sign(dot(normal, direction));
}

float IntersectSphere(const struct Ray *ray, global const struct Object *object) {
    float3 originToSphereCenter = ray->origin - object->position;

    float b = dot(originToSphereCenter, ray->direction);
    float c = dot(originToSphereCenter, originToSphereCenter) - object->radius * object->radius;
    float delta = b * b - c;

    float sqrtDelta =  sqrt(delta);
    float t1 = (-b - sqrtDelta);
    float t2 = (-b + sqrtDelta);

    return fmin(t1, t2);
}

float IntersectPlane(const struct Ray *ray, global const struct Object *object) {
    
    float d = dot(object->position, -object->normal);
    float rayToPlane = dot(ray->origin, object->normal);


    return (rayToPlane - d) / dot(-ray->direction, object->normal);
}

float IntersectDisk(const struct Ray *ray, global const struct Object *object) {
    
    float t = IntersectPlane(ray, object);

    float3 p = ray->origin + ray->direction * t;
    float3 v = p - object->position;
    float d2 = dot(v, v);

    return (d2 <= object->radius * object->radius) * t;
}

float IntersectCube(const struct Ray *ray, global const struct Object *object) {

    float3 inverseDir = 1.0f / ray->direction;

    float tMinX = (object->position.x - ray->origin.x) * inverseDir.x;
    float tMaxX = (object->maxPos.x - ray->origin.x) * inverseDir.x;

    float tMin = fmin(tMinX, tMaxX);
    float tMax = fmax(tMinX, tMaxX);

    float tMinY = (object->position.y - ray->origin.y) * inverseDir.y;
    float tMaxY = (object->maxPos.y - ray->origin.y) * inverseDir.y;

    tMin = fmax(tMin, fmin(tMinY, tMaxY));
    tMax = fmin(tMax, fmax(tMinY, tMaxY));

    float tMinZ = (object->position.z - ray->origin.z) * inverseDir.z;
    float tMaxZ = (object->maxPos.z - ray->origin.z) * inverseDir.z;

    tMin = fmax(tMin, fmin(tMinZ, tMaxZ));
    tMax = fmin(tMax, fmax(tMinZ, tMaxZ));

    bool condition = (tMax >= tMin && tMin > 0.0f);

    return tMin * condition;

}


float3 Reflect(const float3 * incident, const float3 * normal) {
    return *incident - *normal * 2.0f * dot(*incident, *normal);
}

struct Sample FindClosestIntersection(global const struct Object* objects, global const int * numObject, const struct Ray * ray){
    struct Sample sample = {0};
    sample.distance = INFINITY;
    
    for (int i = 0; i < *numObject; ++i) {

        float distance = INFINITY;

        switch(objects[i].type){
            case CUBE:
                distance = IntersectCube(ray, objects + i);
                break;
            case PLANE:
                distance = IntersectPlane(ray, objects + i);
                break;
            case DISK:
                distance = IntersectDisk(ray, objects + i);
                break;
            default:
                distance = IntersectSphere(ray, objects + i);
        }


        if((distance < sample.distance) && (distance>0.0f)){
            sample.distance = distance ;
            sample.point = ray->origin + ray->direction * distance * 1.005f;
            sample.normal = normalize(sample.point - objects[i].position);
            sample.materialID = objects[i].materialID;
        }

    }
    

    return sample;
}


float4 ComputeColor(struct Ray *ray, global const struct Object* objects, global const int * numObject, global const struct Material* materials,  unsigned int *seed) {

    float4 accumulatedColor = 0.0f;
    float4 colorMask = 1.0f;
    float intensity = 1.0f;

    for(int i = 0; i < 8; ++i){
        struct Sample sample = FindClosestIntersection(objects, numObject, ray);
        
        if(sample.distance == INFINITY){
            accumulatedColor += (float4)(0.6f, 0.7f, 0.9f, 1.0f) * intensity;
            break;
        }

        ray->origin = sample.point;

        struct Material material = materials[sample.materialID];

        float3 diffusion = RandomReflection(sample.normal, seed);
        float3 reflection = Reflect(&ray->direction, &sample.normal);

        ray->direction = diffusion + (reflection - diffusion) * material.smoothness;
        
        float lightIntensity = dot(ray->direction, sample.normal);

        float4 emmisionComponent = material.emission * material.emmissionScale;
        float4 diffuseComponent = material.diffuse * material.diffusionScale * lightIntensity;

        accumulatedColor += (2 * emmisionComponent + diffuseComponent/M_PI) * colorMask;
        colorMask *= material.baseColor ;
        intensity *= lightIntensity * 0.5f;
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
global struct Object * objects,
global const int * numObject,
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

    // Simple antialiasing by randomly ofsetting rays
    float3 offset = RandomDirection(&seed);

    float3 pixelPosition = CalculatePixelPosition(x + offset.x - 0.5f, y + offset.y - 0.5f, width, height, camera);

    struct Ray ray;
    ray.origin = camera->position;
    ray.direction = normalize(pixelPosition - ray.origin );

    // Monte - Carlo path tracing have issue with glaring (dark and light spots on consistent color)
    
    float4 finalColor = ComputeColor(&ray, objects, numObject, materials, &seed);

    float scale = 1.0f / (*numFrames+1);
    
    pixels[index] = pixels[index] + (finalColor - pixels[index]) * scale;
}

