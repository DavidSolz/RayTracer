// Structures

struct Material {
    float4 albedo;
    float4 specular;
    float4 emission;
    float metallic;
    float roughness;
    float gloss;
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

    int idA = object->indiceID.x;
    int idB = object->indiceID.y;
    int idC = object->indiceID.z;

    float3 e1 = (vertices[idB] - vertices[idA]);
    float3 e2 = (vertices[idC] - vertices[idA]);

    float3 normal = cross(ray->direction, e2);

    float determinant = dot(e1, normal);

    if( fabs(determinant) < 1e-6f)
        return -1.0f;

    float inverseDeterminant = 1.0f/determinant;
    float3 rayToTriangle = ray->origin - vertices[idA];
    float u = inverseDeterminant * dot(rayToTriangle, normal);

    if( u < 0.0f || u >1.0f)
        return -1.0f;

    float3 q = cross(rayToTriangle, e1);
    float v = inverseDeterminant * dot(ray->direction, q);

    if( v < 0.0f || u+v >1.0f)
        return -1.0f;

    return inverseDeterminant * dot(e2, q);
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

// Negation of Phong illumination reflection formula
float3 Reflect(const float3 incoming, const float3 normal) {

    float3 outgoing = incoming - normal * 2.0f * dot(incoming, normal);

    return normalize(outgoing);
}

// Snells law refraction
float3 Refract(const float3 incoming, const float3 normal, float n1, float n2){

    float cosI = dot(-incoming, normal);
    float sinR2 = (1.0f - cosI * cosI);

    float eta = n1/n2;

    if( eta * sinR2 > 1.0f)
        return Reflect(incoming, -normal);

    float cosR2 = sqrt(1.0f - sinR2 * sinR2);

    float3 direction = incoming * eta + normal * (eta * cosI - cosR2);

    return normalize(direction);
    
}

float SchlickApproximation(const float factor, const float cosAngle){
    return factor + (1.0f - factor) * pow(1.0f - cosAngle, 5.0f);
}

float GGX(const float cosH, const float roughness){

    float alpha = roughness * roughness;
    float cos2H = cosH*cosH;
    float sin2H = sqrt(1.0f - cos2H);
    float denominator = fmax(alpha*cos2H + sin2H, 1e-6f);

    return cos2H/M_PI_F*denominator*denominator;

}

float SmithShlickGGX(const float cosH, const float cosLight, const float cosReflect){
    
    float nominator = 2*cosH*cosReflect;
    float denominator = cosLight + cosReflect;

    return nominator/denominator;

}

float ModifiedFresnel(const float cosAngle, const float factor){
    return 1.0f + (factor - 1.0f)*pow(1.0f-cosAngle, 5.0f);
}

/*

float GlassyBRDF(const float3 normal, const float3 halfVector, const float3 lightVector, const float3 direction, const float3 viewVector, struct Material * material){

    float cosView = (dot(normal, viewVector)+1.0f) *0.5f + 1e-6f;
    float cosLight =  fmax(dot(normal, lightVector), 1e-6f);
    float cosHalfView = fmax(dot(halfVector, viewVector), 0.0f);
    float cosH = fmax(dot(normal, halfVector), 0.0f);
    float cosHO = fmax(dot(halfVector, direction),1e-6f);
    float cosLH = fmax(dot(lightVector, halfVector),1e-6f);

    float D = GGX(cosH, material->roughness);
    float F = SchlickApproximation(material->refractiveIndex, cosHalfView);
    float G = SmithShlickGGX(cosH, cosLH, cosHO);

    return (D * G * F) / (4.0 * cosView * cosView);

}

*/

float GlassyBRDF(const float3 normal, const float3 halfVector, const float3 lightVector, const float3 direction, const float3 viewVector, struct Material * material){

    float cosView = (dot(normal, viewVector)+1.0f) *0.5f + 1e-6f;
    float cosLight =  fmax(dot(normal, lightVector), 1e-6f);
    float cosReflect = fmax(dot(normal, direction), 1e-6f);
    float cosHalfView = fmax(dot(halfVector, viewVector), 0.0f);
    float cosH = fmax(dot(normal, halfVector), 1e-6f);
    float cosHO = fmax(dot(halfVector, direction),1e-6f);
    float cosLH = fmax(dot(lightVector, halfVector),1e-6f);

    float rS = (cosLight - material->refractiveIndex*cosReflect)/(cosLight + material->refractiveIndex*cosReflect);
    float rP = (material->refractiveIndex*cosLight - cosReflect)/(material->refractiveIndex*cosLight + cosReflect);

    float D = GGX(cosH, material->roughness);
    float F = (rS*rS + rP*rP)*0.5f;
    float G = SmithShlickGGX(cosH, cosLH, cosHO);

    return (D * G * F) / (4.0 * cosView);

}

float DiffuseBRDF(const float3 normal, const float3 halfVector, const float3 lightVector, const float3 direction, const float3 viewVector, struct Material * material){

    float cosLight = dot(normal, lightVector);
    float cosReflect = dot(normal, direction);
    float cosHO = dot(halfVector, direction);
    float cosView = fmax(dot(normal, viewVector), 1e-6f);

    float factor = 0.5f + 2.0f * material->roughness * cosHO * cosHO;
    float fresnelIn = ModifiedFresnel(cosLight, factor);
    float fresnelOut = ModifiedFresnel(cosReflect, factor);

    return fresnelIn * fresnelOut * cosReflect * cosView/M_PI_F;
}

float SpecularBRDF(const float3 normal, const float3 halfVector, const float3 lightVector, const float3 direction, const float3 viewVector, struct Material * material){

    float cosIH = (dot(halfVector, lightVector)+1.0f)*0.5f + 1e-6f;
    float cosHO = (dot(halfVector, direction)+1.0f)*0.5f + 1e-6f;

    float specularCoefficient = 1.0f - material->metallic;

    return material->gloss * pow(cosHO, specularCoefficient)/(cosIH*cosHO);
}


float ClearcoatBRDF(const float3 normal, const float3 halfVector, const float3 lightVector, const float3 direction, const float3 viewVector, struct Material * material){

    float cosView = (dot(normal, viewVector)+1.0f) *0.5f + 1e-6f;
    float cosReflect = (dot(normal, direction)+1.0f)*0.5f;
    float cosHO = (dot(halfVector, direction)+1.0f)*0.5f;

    float r0 = 0.04f;
    float alpha = (1.0f - material->gloss)*0.1f + material->gloss * 0.001f;
    float sqrAlpha = alpha * alpha;

    float D = sqrAlpha - 1.0f/( M_PI_F * log10(sqrAlpha)*(1.0f + ( sqrAlpha - 1.0f)*cosHO*cosHO));
    float G = 1.0f;
    float F = ModifiedFresnel(cosReflect, r0);

    return (D * G * F) / (4.0 * cosView);
}

float4 GetSkyBoxColor(const float intensity, const struct Ray * ray){

    const float4 skyColor = (float4)(0.2f, 0.2f, 0.25f, 1.0f);

    return skyColor * intensity;
}

float4 ComputeColor(
    struct Ray * ray,
    const struct Camera * camera,
    global const struct Object * objects,
    global const int * numObject,
    global const struct Material * materials,
    global const float3 * vertices,
    uint * seed
    ){

    float4 accumulatedColor = 0.0f;
    float intensity = 1.0f;
    float4 lightColor = 1.0f;
    float lastRefractance = 1.0f;

    for(int i = 0; i < 8; ++i){
        struct Sample sample = FindClosestIntersection(objects, numObject, ray, vertices);

        if( isinf(sample.length) ){
            //accumulatedColor += GetSkyBoxColor(intensity, ray);
            break;
        }

        ray->origin = sample.point;

        struct Material material =  materials[ sample.materialID ];

        float3 viewVector = normalize(camera->position - sample.point);
        float3 diffusionDirection = DiffuseReflection(sample.normal, seed);
        float3 reflectionDirection = Reflect(ray->direction, sample.normal);
        float3 refractionDirecton = Refract(ray->direction, sample.normal, lastRefractance, material.refractiveIndex);

        float3 lightVector = -ray->direction;
        float3 halfVector = normalize((sample.normal + reflectionDirection)*0.5f);

        float3 direction = mix(diffusionDirection, reflectionDirection, material.metallic);
        ray->direction = normalize(mix(direction, refractionDirecton, material.transparency ));

        float cosLight = (dot(lightVector, sample.normal)+1.0f)*0.5f;
        float cosView = fmax(dot(sample.normal, viewVector), 0.0f);

        float4 emissionComponent = 2 * material.emission * material.emissionScale * intensity * cosView;
        float4 diffusionComponent = material.albedo * DiffuseBRDF(sample.normal, halfVector, lightVector, ray->direction, viewVector, &material) * (1.0f - material.metallic) * (1.0f - material.transparency); 
        float4 glassyComponent = material.albedo * GlassyBRDF(sample.normal, halfVector, lightVector, ray->direction, viewVector, &material) * (1.0f - material.metallic) * material.transparency;
        //float4 clearcoatComponent = material.roughness * material.albedo * ClearcoatBRDF(sample.normal, halfVector, lightVector, ray->direction, viewVector, &material);
        float4 specularComponent = material.albedo * SpecularBRDF(sample.normal, halfVector, lightVector, ray->direction, viewVector, &material);

        accumulatedColor += diffusionComponent + glassyComponent + emissionComponent + specularComponent;
        intensity *= cosLight;
        lightColor *= diffusionComponent;
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
    float3 pixelPosition = CalculatePixelPosition(x + offset.x - 0.5f, y + offset.y - 0.5f, width, height, &camera);

    private struct Ray ray;
    ray.origin = camera.position;
    ray.direction = normalize(pixelPosition - ray.origin );

    // Monte - Carlo path tracing have issue with glaring (dark and light spots on consistent color)

    // Supersampling approach results in no noise but very low amount of fps
    float4 sample = ComputeColor(&ray, &camera, objects, numObject, materials, vertices, &seed);

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
