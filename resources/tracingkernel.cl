// Structures

struct Material {
    float4 baseColor;
    float4 diffuse;
    float4 specular;
    float4 emission;
    float smoothness;
    float emmissionScale;
    float diffusionScale;
    float transparency;
    float refractiveIndex;
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
    float aspectRatio;
    float near;
    float far;
    float fov;
    float pitch;
    float yaw;

};

// Functions

// PCG_Hash
float Rand(unsigned int * seed){
    *seed = *seed *747796405u + 2891336453u;
    unsigned int word = ((*seed >> ((*seed >> 28u) + 4u)) ^ *seed) * 277803737u;
    return ((word>>22u) ^ word)/(float)UINT_MAX;
}

float3 RandomDirection(unsigned int *seed){
    float latitude = acos(2.0f * Rand(seed) - 1.0f) - 3.1415926353f/2.0f;
    float longitude = Rand(seed) * 2 *  3.1415926535f;

    float cosLatitude = cos(latitude);

    return (float3)(
        cosLatitude * cos(longitude),
        cosLatitude * sin(longitude),
        sin(latitude)
    );
}

float3 RandomReflection(float3 normal, unsigned int *seed){
    float3 direction = RandomDirection(seed);
    return direction * sign( dot(normal, direction) );
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

    float3 p = ray->origin + ray->direction * t * 1.000005f;
    float3 v = p - object->position;
    float d2 = dot(v, v);

    bool condition = (d2 <= object->radius * object->radius);

    return t * condition - 1 + condition;
}

float IntersectCube(const struct Ray *ray, global const struct Object *object) {
    
    float3 dirs = (float3)(sign(ray->direction.x), sign(ray->direction.y), sign(ray->direction.z) );

    float3 values = (float3)(fabs(ray->direction.x), fabs(ray->direction.y), fabs(ray->direction.z));

    values.x = fmax(values.x, 0.00000001f);
    values.y = fmax(values.y, 0.00000001f);
    values.z = fmax(values.z, 0.00000001f);

    float3 direction = 1.0f/(values * dirs);

    float tMin = (object->position.x - ray->origin.x) * direction.x;
    float tMax = (object->maxPos.x - ray->origin.x) * direction.x;

    float min = fmin(tMin, tMax);
    float max = fmax(tMin, tMax);

    tMin = min;
    tMax = max;

    float tyMin = (object->position.y - ray->origin.y) * direction.y;
    float tyMax = (object->maxPos.y - ray->origin.y) * direction.y;

    min = fmin(tyMin, tyMax);
    max = fmax(tyMin, tyMax);

    tyMin = min;
    tyMax = max;

    if ((tMin > tyMax) || (tyMin > tMax)) {
        return -1.0f;  
    }

    tMin = fmax(tMin, tyMin);
    tMax = fmin(tMax, tyMax);

    float tzMin = (object->position.z - ray->origin.z) * direction.z;
    float tzMax = (object->maxPos.z - ray->origin.z) * direction.z;

    min = fmin(tzMin, tzMax);
    max = fmax(tzMin, tzMax);

    tzMin = min;
    tzMax = max;
    
    if ((tMin > tzMax) || (tzMin > tMax)) {
        return -1.0f; 
    }

    tMin = fmax(tMin, tzMin);
    tMax = fmin(tMax, tzMax);

    if (tMin > 0.0f) {
        return tMin;
    }

    return -1.0f;
}

float3 ComputeBoxNormal(global const float3 * nearVertice, global const float3 * farVertice, const float3 * intersection){
    float3 boxCenter = (*nearVertice + *farVertice) * 0.5f;
    float3 localIntersection = *intersection - boxCenter;
    return normalize(localIntersection);
}

struct Sample FindClosestIntersection(global const struct Object* objects, global const int * numObject, const struct Ray * ray){
    struct Sample sample = {0};
    sample.distance = INFINITY;

    for (int i = 0; i < *numObject; ++i) {

        float distance = -1.0f;

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

        if((distance < sample.distance) && (distance>0.1f)){
            sample.distance = distance ;
            sample.point = ray->origin + ray->direction * distance * 1.000005f;
            sample.materialID = objects[i].materialID;

            switch(objects[i].type){

                case CUBE:
                    sample.normal = ComputeBoxNormal(&objects[i].position, &objects[i].maxPos, &sample.point);
                    break;

                case SPHERE:
                    sample.normal = normalize(sample.point - objects[i].position);
                    break;

                default:
                    sample.normal = objects[i].normal;
            }
            
        }

    }

    return sample;
}

// Negation of Phong illumination reflection formula

float3 Reflect(const float3 * incident, const float3 * normal) {
    return *incident - *normal * 2.0f * dot(*incident, *normal);
}

// Snells law refraction

float3 Refract(const float3 * incident, const float3 * normal, const struct Material * material){

    float reflectance  = 1.0f/material->refractiveIndex;

    float cosI = -dot(*incident, *normal);
    float sinR2 = reflectance * reflectance * (1.0f - cosI * cosI);

    float cosR = sqrt(1.0f - sinR2);

    return (*incident * reflectance) + *normal * ( reflectance * cosI - cosR);
}

float4 GetSkyBoxColor(const float intensity, const struct Ray * ray){
    float t = intensity/fmax(ray->direction.y + 1.0f, 0.00001f);

    const float4 skyColor = (float4)(0.5f, 0.7f, 1.0f, 1.0f);

    return skyColor * t;
}

float4 ComputeColor(struct Ray *ray, global const struct Object* objects, global const int * numObject, global const struct Material* materials,  unsigned int *seed) {

    float4 accumulatedColor = 0.0f;
    float4 colorMask = 1.0f;
    float intensity = 1.0f;
    float lastRefrectance = 1.00029f;

    for(int i = 0; i < 8; ++i){
        struct Sample sample = FindClosestIntersection(objects, numObject, ray);

        if(sample.distance == INFINITY){
            accumulatedColor += GetSkyBoxColor(intensity, ray);
            break;
        }

        ray->origin = sample.point;

        struct Material material =  materials[sample.materialID];

        bool isRefraction = ( material.transparency < Rand(seed) );

        float3 diffusionDirection = RandomReflection(sample.normal, seed);
        float3 reflectionDirection = Reflect(&ray->direction, &sample.normal);
        float3 refractionDirection = Refract(&ray->direction, &sample.normal, &material);

        ray->direction = diffusionDirection + (reflectionDirection - diffusionDirection) * material.smoothness;
        ray->direction = normalize(ray->direction);

        ray->origin += ray->direction * 0.01f;

        float lightIntensity = dot(ray->direction, sample.normal);

        float4 emmisionComponent = normalize(material.emission * material.emmissionScale) ;
        float4 reflectionComponent = normalize(material.diffuse * material.diffusionScale * 2 * lightIntensity);

        accumulatedColor += (2 * emmisionComponent + reflectionComponent/M_PI_F) * colorMask;
        colorMask *= sqrt(material.baseColor);
        intensity *= lightIntensity * 0.1f;
    }

    return accumulatedColor;
}

float3 CalculatePixelPosition(const int x, const int y, const int width, const int height, global const struct Camera * camera){

    float tanHalfFOV = tan(radians(camera->fov) * 0.5f);
    float pixelXPos = (2.0 * x / width - 1.0f) * camera->aspectRatio * tanHalfFOV * camera->near;
    float pixelYPos = (2.0 * y / height - 1.0f) * tanHalfFOV * camera->near;

    return camera->position + camera->front * camera->near + camera->right * pixelXPos + camera->up * pixelYPos;
}

// Main


void kernel RayTrace(
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
    unsigned int seed = (*numFrames<<16) ^ (*numFrames >>13) + index;

    // Simple anti-aliasing techinque 
    float3 offset = RandomDirection(&seed);
    float3 pixelPosition = CalculatePixelPosition(x + offset.x - 0.5f, y + offset.y - 0.5f, width, height, camera);

    pixelPosition.x = fmax(pixelPosition.x, 0.00001f);
    pixelPosition.y = fmax(pixelPosition.y, 0.00001f);
    pixelPosition.z = fmin(pixelPosition.z, 0.00001f);

    struct Ray ray;
    ray.origin = camera->position;
    ray.direction = normalize(pixelPosition - ray.origin );

    // Monte - Carlo path tracing have issue with glaring (dark and light spots on consistent color)

    // Supersampling approach results in no noise but very low amount of fps

    float4 sample = ComputeColor(&ray, objects, numObject, materials, &seed);

    float scale = 1.0f / (*numFrames+1);

    pixels[index] =  pixels[index] + (sample - pixels[index]) * scale;
}

