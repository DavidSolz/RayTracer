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
    int insideObject;
};

struct Object{
    enum SpatialType type;
    float radius;
    float3 position;
    float3 normal;
    float3 maxPos;
    uint materialID;
};

struct Sample{
    float length;
    float3 point;
    float3 normal;
    uint materialID;
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
float Rand(uint * seed){
    *seed = *seed *747796405u + 2891336453u;
    uint word = ((*seed >> ((*seed >> 28u) + 4u)) ^ *seed) * 277803737u;
    return ((word>>22u) ^ word)/(float)UINT_MAX;
}

float3 RandomDirection(uint *seed){
    float latitude = acos(2.0f * Rand(seed) - 1.0f) - 3.1415926353f/2.0f;
    float longitude = Rand(seed) * 2 *  3.1415926535f;

    float cosLatitude = cos(latitude);

    return (float3)(
        cosLatitude * cos(longitude),
        cosLatitude * sin(longitude),
        sin(latitude)
    );
}

float3 RandomReflection(float3 normal, uint *seed){
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

    float3 p = ray->origin + ray->direction * t * 1.000001f;
    float3 v = p - object->position;
    float d2 = dot(v, v);

    bool condition = islessequal(d2, object->radius * object->radius);

    return t * condition - 1 + condition;
}

float IntersectCube(const struct Ray *ray, global const struct Object *object) {
    
    float3 dirs = sign(ray->direction);
    float3 values = fabs(ray->direction);

    const float3 minimalBias = 1e-6f;

    values = fmax(values, minimalBias);

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

    if ( isgreater(tMin, tyMax) || isgreater(tyMin, tMax)) {
        return INFINITY;  
    }

    tMin = fmax(tMin, tyMin);
    tMax = fmin(tMax, tyMax);

    float tzMin = (object->position.z - ray->origin.z) * direction.z;
    float tzMax = (object->maxPos.z - ray->origin.z) * direction.z;

    min = fmin(tzMin, tzMax);
    max = fmax(tzMin, tzMax);

    tzMin = min;
    tzMax = max;
    
    if ( isgreater(tMin, tzMax) || isgreater(tzMin, tMax)) {
        return INFINITY; 
    }

    tMin = fmax(tMin, tzMin);
    tMax = fmin(tMax, tzMax);

    if ( isgreater(tMin, 0.0f) ) {
        return tMin;
    }

    return INFINITY;
}

float3 ComputeBoxNormal(global const float3 * nearVertice, global const float3 * farVertice, const float3 * intersectionPoint){
    
    float3 boxCenter = (*farVertice + *nearVertice)*0.5f;
    float3 radius = (*farVertice - *nearVertice)*0.5f;
    float3 pointToCenter = *intersectionPoint - boxCenter;

    const float bias = 1.000001f;

    float3 normal = sign(pointToCenter) * step(fabs(fabs(pointToCenter) - radius), bias);

    return normalize(normal);
}

struct Sample FindClosestIntersection(global const struct Object* objects, global const int * numObject, const struct Ray * ray){

    struct Sample sample = {0};
    sample.length = INFINITY;

    for (int i = 0; i < *numObject; ++i) {

        float length = INFINITY;

        switch(objects[i].type){
            case CUBE:
                length = IntersectCube(ray, objects + i);
                break;
            case PLANE:
                length = IntersectPlane(ray, objects + i);
                break;
            case DISK:
                length = IntersectDisk(ray, objects + i);
                break;
            default:
                length = IntersectSphere(ray, objects + i);
        }

        if( isless(length, sample.length) && isgreater(length, 0.1f) ){
            sample.length = length ;
            sample.point = mad(ray->direction, length, ray->origin);
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

float3 Refract(const float3 * incident, const float3 * normal, const float n1, const float n2){

    float reflectance  = (n1-n2) / (n1+n2);

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

float4 ComputeColor(struct Ray *ray, global const struct Object* objects, global const int * numObject, global const struct Material* materials,  uint *seed) {

    float4 accumulatedColor = 0.0f;
    float4 colorMask = 1.0f;
    float intensity = 1.0f;

    for(int i = 0; i < 8; ++i){
        struct Sample sample = FindClosestIntersection(objects, numObject, ray);

        if( isinf(sample.length) ){
            accumulatedColor += GetSkyBoxColor(intensity, ray);
            break;
        }

        ray->origin = sample.point;

        struct Material material =  materials[sample.materialID];

        float3 diffusionDirection = RandomReflection(sample.normal, seed);
        float3 reflectionDirection = Reflect(&ray->direction, &sample.normal);

        ray->direction = mix(diffusionDirection, reflectionDirection , material.smoothness);

        float lightIntensity = clamp(dot(ray->direction, sample.normal), 0.0f, 1.0f);

        float4 emmisionComponent = material.emission * material.emmissionScale * 2;
        float4 reflectionComponent = material.diffuse * material.diffusionScale * 2 * lightIntensity * (1.0f/3.1415926535f);

        accumulatedColor += (emmisionComponent +  reflectionComponent) * colorMask;
        colorMask *= material.baseColor;
        intensity *= lightIntensity * 0.01f;
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

    uint x = get_global_id(0);
    uint y = get_global_id(1);

    uint width = get_global_size(0);
    uint height = get_global_size(1);

    uint index = y * width + x;
    uint seed = (*numFrames<<16) ^ (*numFrames >>13) + index;

    // Simple anti-aliasing techinque 
    float3 offset = RandomDirection(&seed);
    float3 pixelPosition = CalculatePixelPosition(x, y, width, height, camera);

    struct Ray ray;
    ray.origin = camera->position;
    ray.direction = normalize(pixelPosition - ray.origin );

    // Monte - Carlo path tracing have issue with glaring (dark and light spots on consistent color)

    // Supersampling approach results in no noise but very low amount of fps
    float4 sample = ComputeColor(&ray, objects, numObject, materials, &seed);

    float scale = 1.0f / (*numFrames + 1);

    pixels[index] =  mix(pixels[index], sample, scale);
}

void kernel AntiAlias(global float4 * input, global float4 * output){
    
    int x = get_global_id(0);
    int y = get_global_id(1);

    int width = get_global_size(0);
    int height = get_global_size(1);

    int index = y * width + x;

    float4 pixelValue = 0.0f;

    constant float matrix[3][3] = {
            {-0.2f, 2.0f, -0.2f},
            {0.8f, 1.0f, 0.8f},
            {-0.2f, 2.0f, -0.2f}
    };

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            uint neighborX = clamp(x + i, 0, width - 1);
            uint neighborY = clamp(y + j, 0, height - 1);
            float4 neighborValue = input[neighborY * width + neighborX];

            float weight = matrix[i + 1][j + 1]/8.0f;
            pixelValue += weight * neighborValue;
        }
    }

    output[index] = pixelValue;
    
}