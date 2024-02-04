// Structures

struct Material {
    float4 baseColor;
    float4 specular;
    float4 emission;
    float metallic;
    float roughness;
    float emissionScale;
    float diffusionScale;
    float transparency;
    float refractiveIndex;
} __attribute__((aligned(128)));

enum SpatialType{
    SPHERE,
    PLANE,
    DISK,
    CUBE,
    TRIANGLE
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
    float3 indiceID;
    uint materialID;
} __attribute((aligned(128)));

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
    float rotationSpeed;
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
    float latitude = acos(2.0f * Rand(seed) - 1.0f) - M_PI/2.0f;
    float longitude = Rand(seed) * 2 *  3.1415926535f;

    float cosLatitude = cos(latitude);

    return normalize((float3)(
        cosLatitude * cos(longitude),
        cosLatitude * sin(longitude),
        sin(latitude)
    ));
}

float3 DiffuseReflection(float3 normal, uint *seed){
    float3 direction = RandomDirection(seed);
    return direction * sign( dot(normal, direction) );
}

float IntersectSphere(const struct Ray * ray, global const struct Object *object) {
    float3 originToSphereCenter = ray->origin - object->position;

    float b = dot(originToSphereCenter, ray->direction);
    float c = dot(originToSphereCenter, originToSphereCenter) - object->radius * object->radius;
    float delta = b * b - c;

    float sqrtDelta =  sqrt(delta);
    float t1 = (-b - sqrtDelta);
    float t2 = (-b + sqrtDelta);

    return fmin(t1, t2);
}

float IntersectPlane(const struct Ray * ray, global const struct Object *object) {

    float d = dot(object->position, object->normal);
    float rayToPlane = dot(ray->origin, object->normal);

    return (rayToPlane - d) / dot(-ray->direction, object->normal);
}

float IntersectDisk(const struct Ray * ray, global const struct Object *object) {

    float t = IntersectPlane(ray, object);

    if( fabs(t) < 1e-6f)
        return -1.0f;

    float3 p = ray->origin + ray->direction * t * 1.000001f;
    float3 v = p - object->position;
    float d2 = dot(v, v);

    if (d2 <= object->radius * object->radius )
        return t;

    return -1.0f;

}

float IntersectCube(const struct Ray * ray, global const struct Object *object) {

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

    if ( tMin > tzMax || tzMin > tMax) {
        return -1.0f;
    }

    tMin = fmax(tMin, tzMin);
    tMax = fmin(tMax, tzMax);

    if ( tMin > 0.0f ) {
        return tMin;
    }

    return -1.0f;
}

float IntersectTriangle(
    const struct Ray * ray,
    global const struct Object *object,
    global const float3 * vertices
    ) {

    const float epsilon = 1e-6f;

    int idA = object->indiceID.x;
    int idB = object->indiceID.y;
    int idC = object->indiceID.z;

    float3 e1 = (vertices[idB] - vertices[idA]);
    float3 e2 = (vertices[idC] - vertices[idA]);

    float3 normal = cross(ray->direction, e2);
    float det = dot(e1, normal);

    if( fabs(det) < epsilon)
        return -1.0f;

    float f = 1.0f/det;
    float3 rayToTriangle = ray->origin - vertices[idA];
    float u = f * dot(rayToTriangle, normal);

    if( u < 0.0f || u >1.0f)
        return -1.0f;

    float3 q = cross(rayToTriangle, e1);
    float v = f *dot(ray->direction, q);

    if( v < 0.0f || u+v >1.0f)
        return -1.0f;

    return f * dot(e2, q);
}

float3 ComputeBoxNormal(
    global const float3 * nearVertice, 
    global const float3 * farVertice, 
    const float3 * intersectionPoint
    ){

    const float epsilon = 1.000001f;

    float3 boxCenter = (*farVertice + *nearVertice)*0.5f;
    float3 radius = (*farVertice - *nearVertice)*0.5f;
    float3 pointToCenter = *intersectionPoint - boxCenter;

    float3 normal = sign(pointToCenter) * step(fabs(fabs(pointToCenter) - radius), epsilon);

    return normalize(normal);
}


struct Sample FindClosestIntersection(
    global const struct Object * objects,
    global const int * numObject,
    const struct Ray * ray,
    global const float3 * vertices){

    struct Sample sample = {0};
    sample.length = INFINITY;

    for (int i = 0; i < *numObject; ++i) {

        float length = -1.0f;

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
            case TRIANGLE:
                length = IntersectTriangle(ray, objects + i, vertices);
                break;
            default:
                length = IntersectSphere(ray, objects + i);
        }

        if( (length < sample.length) && (length > 0.1f) ){
            sample.length = length ;
            sample.point = ray->origin + ray->direction * length;
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
                    break;
            }

        }

    }

    return sample;
}

// Schlick Approximation
float3 FresnelSchlick(const float3 direction, float cosTheta) {
    return normalize(direction + (1.0f - direction) * pow(1.0f - cosTheta, 5.0f));
}

// Negation of Phong illumination reflection formula
float3 SpecularReflect(const float3 incoming, const float3 normal) {
    return incoming - normal * 2.0f * dot(incoming, normal);
}

// Snells law refraction
float3 Refract(const float3 incoming, const float3 normal, float n1, float n2){

    float cosI = dot(-incoming, normal);
    float sinR2 = (1.0f - cosI * cosI);

    float eta = n1/n2;

    if( eta * sinR2 > 1.0f)
        return SpecularReflect(incoming, normal);

    float cosR2 = sqrt(1.0f - sinR2 * sinR2);

    float3 direction = incoming * eta + normal * (eta * cosI - cosR2);

    return normalize(direction);
    
}



float4 GetSkyBoxColor(const float intensity, const struct Ray * ray){
    //const float4 skyColor = (float4)(ray->direction.x, ray->direction.y, ray->direction.z, 1.0f);

    const float4 skyColor = (float4)(0.2f, 0.2f, 0.25f, 1.0f);

    return skyColor * intensity;
}

// Stack definition

#define MAX_STACK_SIZE 10

struct Stack {
    float4 elements[MAX_STACK_SIZE];
    int top;
};

void Push(private struct Stack * stack, float4 element) {
    if (stack->top < MAX_STACK_SIZE) {
        stack->elements[stack->top++] = element;
    }
}

float4 Pop(private struct Stack * stack) {
    if (stack->top > 0) {
        return stack->elements[--stack->top];
    }
    return (float4)(0.0f, 0.0f, 0.0f, 0.0f);
}

float4 ComputeColor(
    struct Ray * ray,
    global const struct Object * objects,
    global const int * numObject,
    global const struct Material * materials,
    uint * seed,
    global const float3 * vertices
    ){

    float4 accumulatedColor = 0.0f;
    float4 lightColor = 1.00f;
    float intensity = 1.0f;
    float lastRefractance = 1.00029f; //IOR for air

    for(int i = 0; i < 8; ++i){
        struct Sample sample = FindClosestIntersection(objects, numObject, ray, vertices);

        if( isinf(sample.length) ){
            accumulatedColor += GetSkyBoxColor(intensity, ray);
            break;
        }

        ray->origin = sample.point;

        struct Material material =  materials[ sample.materialID ];

        float3 reflectionDirection = SpecularReflect(ray->direction, sample.normal);
        float3 diffusionDirection = normalize(RandomDirection(seed) + sample.normal);
        float3 refractionDirecton = Refract(ray->direction, sample.normal, lastRefractance, material.refractiveIndex);

        float3 direction = mix(diffusionDirection, reflectionDirection, material.metallic);
        ray->direction = mix(direction, refractionDirecton, material.transparency);

        float lightIntensity = clamp(dot(-ray->direction, sample.normal), 0.0f, 1.0f);

        float4 specularComponent = material.baseColor * pow( (-lightIntensity+1.0f)*0.5f, 1.0f/material.roughness) ;
        float4 emissionComponent = material.emission * exp(material.emission);
        float4 diffusionComponent = material.baseColor * lightIntensity * (1.0f - material.roughness);

        accumulatedColor += (2 * emissionComponent + (specularComponent)/M_PI_F) * lightColor;

        lightColor *= material.baseColor;
        intensity *= lightIntensity ;
        lastRefractance = material.refractiveIndex;
    }

    return accumulatedColor;
}

float3 CalculatePixelPosition(
    const int x, 
    const int y, 
    const int width, 
    const int height, 
    const struct Camera * camera
    ){

    float tanHalfFOV = tan(radians(camera->fov) * 0.5f);
    float pixelXPos = (2.0 * x / width - 1.0f) * camera->aspectRatio * tanHalfFOV * camera->near;
    float pixelYPos = (2.0 * y / height - 1.0f) * tanHalfFOV * camera->near;

    return camera->position + camera->front * camera->near + camera->right * pixelXPos + camera->up * pixelYPos;
}


// Main

void kernel RayTrace(
    global float4* pixels,
    global struct Object * objects,
    global struct Material * materials,
    global const float3 * vertices,
    global const int * numObject,
    struct Camera camera,
    int numFrames
    ){

    uint x = get_global_id(0);
    uint y = get_global_id(1);

    uint width = get_global_size(0);
    uint height = get_global_size(1);

    uint index = y * width + x;
    uint seed = (numFrames<<16) ^ (numFrames >>13) + index;

    // Simple anti-aliasing techinque
    float3 offset = RandomDirection(&seed);
    float3 pixelPosition = CalculatePixelPosition(x + offset.x + 0.5f, y + offset.y + 0.5f, width, height, &camera);

    private struct Ray ray;
    ray.origin = camera.position;
    ray.direction = normalize(pixelPosition - ray.origin );

    // Monte - Carlo path tracing have issue with glaring (dark and light spots on consistent color)

    // Supersampling approach results in no noise but very low amount of fps
    float4 sample = ComputeColor(&ray, objects, numObject, materials, &seed, vertices);

    float scale = 1.0f / (numFrames + 1);

    pixels[index] = mix(pixels[index], sample, scale);
}

void kernel AntiAlias(global float4 * input){

    int x = get_global_id(0);
    int y = get_global_id(1);

    int width = get_global_size(0);
    int height = get_global_size(1);

    int index = y * width + x;

    float4 pixelValue = input[index];

    const float matrix[3][3] = {
            {-0.1f, 0.5f, -0.1f},
            {0.5f, 1.0f, 0.5f},
            {-0.1f, 0.5f, -0.1f}
    };

    // for (int i = -1; i <= 1; i++) {
    //     for (int j = -1; j <= 1; j++) {
    //         int neighborX = clamp(x + i, 0, width - 1);
    //         int neighborY = clamp(y + j, 0, height - 1);

    //         int idx = neighborY * width + neighborX;

    //         pixelValue = fmax(pixelValue, input[idx] * matrix[i+1][j+1]);

    //     }
    // }

    input[index] = pixelValue;

}
