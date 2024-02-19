
#include "resources/kernels/KernelStructs.h"
// Defines

#define ONE_OVER_PI 1.0f/M_PI_F
#define ONE_OVER_2_PI 1.0f/M_PI_2_F
#define PI_OVER_TWO M_PI_F/2.0f;

#define EPSILON 1.000001f

// Functions

float Rand(uint * seed){
    *seed = *seed * 747796405u + 2891336453u;
    uint word = ((*seed >> ((*seed >> 28u) + 4u)) ^ *seed) * 277803737u;
    return ((word>>22u) ^ word)/(float)UINT_MAX;
}

float3 RandomDirection(uint * seed){
    float latitude = ( 2.0f * Rand(seed) - 1.0f ) * M_PI_F;
    float longitude = Rand(seed) * M_PI_F;

    float cosLatitude = cos(latitude);

    return normalize((float3)(
        cosLatitude * cos(longitude),
        cosLatitude * sin(longitude),
        sin(latitude)
    ));
}

float IntersectSphere(const struct Ray * ray, const struct Object * object) {
    float3 originToSphereCenter = ray->origin - object->position;

    float b = dot(originToSphereCenter, ray->direction);
    float c = dot(originToSphereCenter, originToSphereCenter) - object->radius * object->radius;
    float delta = b * b - c;

    float sqrtDelta =  sqrt(delta);
    float t1 = (-b - sqrtDelta);
    float t2 = (-b + sqrtDelta);

    return fmin(t1, t2);
}

float IntersectPlane(const struct Ray * ray, const struct Object * object) {

    float d = dot(object->position, object->normal);
    float rayToPlane = dot(ray->origin, object->normal);

    return (rayToPlane - d) / dot(-ray->direction, object->normal);
}

float IntersectDisk(const struct Ray * ray, const struct Object * object) {

    float t = IntersectPlane(ray, object);

    if( fabs(t) < 1e-6f)
        return -1.0f;

    float3 p = ray->origin + ray->direction * t * EPSILON;
    float3 v = p - object->position;
    float d2 = dot(v, v);

    if (d2 <= object->radius * object->radius )
        return t;

    return -1.0f;

}

float IntersectCube(const struct Ray * ray, const struct Object * object) {

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
    const struct Object * object,
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
    const float3 nearVertice,
    const float3 farVertice,
    const float3 intersectionPoint
    ){

    float3 boxCenter = (farVertice + nearVertice)*0.5f;
    float3 radius = (farVertice - nearVertice)*0.5f;
    float3 pointToCenter = intersectionPoint - boxCenter;

    float3 normal = sign(pointToCenter) * step(fabs(fabs(pointToCenter) - radius), EPSILON);

    return normalize(normal);
}

struct Sample FindClosestIntersection(
    const struct Resources resources, 
    const struct Ray * ray
    ){

    global const struct Object * objects = resources.objects;
    global const float3 * vertices = resources.vertices;
    int numObject = resources.numObject;

    struct Sample sample = {0};
    sample.length = INFINITY;

    for (int id = 0; id < numObject; ++id) {

        struct Object object = objects[id];

        float length = -1.0f;

        switch(object.type){
            case CUBE:
                length = IntersectCube(ray, &object);
                break;
            case PLANE:
                length = IntersectPlane(ray, &object);
                break;
            case DISK:
                length = IntersectDisk(ray, &object);
                break;
            case TRIANGLE:
                length = IntersectTriangle(ray, &object, vertices);
                break;
            default:
                length = IntersectSphere(ray, &object);
        }

        if( (length < sample.length) && (length > 0.01f) ){
            sample.length = length ;
            sample.point = ray->origin + ray->direction * length * EPSILON;
            sample.objectID = id;
            sample.materialID = object.materialID;

            switch(object.type){

                case CUBE:
                    sample.normal = ComputeBoxNormal(object.position, object.maxPos, sample.point);
                    break;

                case SPHERE:
                    sample.normal = normalize(sample.point - object.position);
                    break;

                default:
                    sample.normal = normalize(object.normal);
                    break;
            }

        }

    }

    return sample;
}

float3 Reflect(const float3 incoming, const float3 normal) {
    float3 outgoing = incoming - normal * 2.0f * dot(incoming, normal);
    return normalize(outgoing);
}

float3 DiffuseReflect(const float3 normal, uint * seed){

    float3 randomDirection = RandomDirection(seed);
    float cosDirection = dot(normal, randomDirection);

    return normalize(randomDirection * cosDirection + normal);
}

float3 Refract(
    const float3 incoming, 
    float3 normal, 
    const float n1, 
    const float n2
    ){

    float cosI = fmin(dot(-incoming, normal), 1.0f);
    float sinI = sqrt(1.0f - cosI*cosI);
    float eta = n1/n2;

    if( eta * sinI > 1.0f)
        return Reflect(incoming, normal);

    float3 perpendicularRay =  eta * (incoming + cosI * normal);
    float rayLength = length(perpendicularRay);
    float3 parallelRay = -sqrt(1.0f - rayLength*rayLength) * normal;

    return normalize(perpendicularRay + parallelRay);
}

float IORToR0(const float indexOfRefraction){
    float R0 = (1.0f - indexOfRefraction) / (1.0f + indexOfRefraction);
    return R0 * R0;
}

float4 SchlickFresnel(const float cosHalf, const float4 F0){
    return F0 + (1.0f - F0) * pow(1.0f - cosHalf, 5.0f);
}

float GGX(const float cosHalf, const float roughness){
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float cosHalf4 = cosHalf*cosHalf*cosHalf*cosHalf;
    return ONE_OVER_PI * alpha2 / (cosHalf4 * (alpha2 - 1.0f) + 1.0f);
}

float SchlickGGX(const float cosDirection, const float roughness){
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;

    float nominator = 2.0f * cosDirection;
    float denominator = cosDirection + sqrt(alpha2 + (1.0f - alpha2) * cosDirection * cosDirection);

    return fmin(nominator / denominator, 1.0f);
}

float GeometricSmithShlickGGX(
    const float cosView, 
    const float cosLight, 
    const float roughness
    ){

    float ggx1 = SchlickGGX(cosLight, roughness);
    float ggx2 = SchlickGGX(cosView, roughness);

    return ggx1*ggx2;
}

float4 physicalBRDF(
    const float3 normal, 
    const float3 halfVector, 
    const float3 lightVector, 
    const float3 outgoing, 
    const float3 viewVector, 
    struct Material * material
    ){

    float cosView = fmax(dot(normal, viewVector), 0.0f);
    float cosLight =  fmax(dot(normal, lightVector), 0.0f);

    float cosHalf = dot(normal, halfVector);
    float cosHalfView = fmax(dot(halfVector, viewVector), 0.0f);

    float4 R0 = IORToR0(material->indexOfRefraction);

    float D = GGX(cosHalf, material->roughness);
    float G = GeometricSmithShlickGGX(cosView, cosLight, material->roughness);
    float4 F = SchlickFresnel(cosHalfView, R0);

    float denominator = 4.0f * cosView * cosLight + 1e-6f; 

    return (D * F * G) / denominator;

}

float4 GetTexturePixel(
    global const float4 * texture,
    const struct Object * object,
    const struct Texture textureInfo,
    const float3 localPoint,
    const float3 normal
    ){

    float4 color = 0.0f;
    int texelX = 0;
    int texelY = 0;

    if( object->type == CUBE ){

        float3 boxSize = object->maxPos - object->position;
        float3 relativePos = (localPoint - object->position) / boxSize;

        float u, v;

        float conditionU = fabs(normal.x) == 1.0f;
        float conditionV = fabs(normal.y) == 1.0f;

        if( fabs(normal.x) == 1.0f){
            v = relativePos.z;
        }else if( fabs(normal.y) == 1.0f ){
            v = relativePos.z;
        }else{
            v = relativePos.y;
        }

        u = conditionU * relativePos.y + (1.0f - conditionU) * relativePos.x;
        v = (conditionV+conditionU) * relativePos.z + (1.0f - conditionV - conditionU) * relativePos.y;

        texelX = floor(u * textureInfo.width);
        texelY = floor(v * textureInfo.height);

        int idx = textureInfo.offset + (textureInfo.height - 1 - texelY) * textureInfo.width + texelX;

        color = texture [ idx ];

    } else if ( object->type == DISK ){

        float3 pointToCenter = localPoint - object->position;
        float theta = atan2(pointToCenter.y, pointToCenter.x);
        float radius = length( pointToCenter );

        float u = (theta + M_PI) / M_PI_2_F;
        float v = (radius / object->radius);

        texelX = floor( u * textureInfo.width );
        texelY = floor( v * textureInfo.height ); 

        int idx = textureInfo.offset + (textureInfo.height - 1 - texelY) * textureInfo.width + texelX;

        color = texture [ idx ];

    } else if ( object->type == SPHERE ){

        float3 pointToCenter = normalize(localPoint - object->position);

        float theta = acos( pointToCenter.z );
        float phi = atan2(pointToCenter.y, pointToCenter.x);

        float u = phi / M_PI_2_F;
        float v = 1.0f -  theta / M_PI_F;

        texelX = floor( u * textureInfo.width );
        texelY = floor( v * textureInfo.height ); 

        int idx = textureInfo.offset + (textureInfo.height - 1 - texelY) * textureInfo.width + texelX;

        color = texture [ idx ];
    }

    return color;
}

float4 ComputeColor(
    const struct Resources resources, 
    struct Ray * ray, 
    const struct Camera * camera, 
    uint * seed
    ){

    float4 accumulatedColor = 0.0f;
    float4 lightColor = 1.0f;
    float lastIOR = 1.0f;

    const float4 skyColor = (float4)(0.2f, 0.2f, 0.25f, 1.0f);
    global const struct Material * materials = resources.materials;
    global const struct Object * objects = resources.objects;
    global const struct Texture * infos = resources.textureInfo;
    global const float4 * texture = resources.textureData;

    for(int i = 0; i < 8; ++i){
        struct Sample sample = FindClosestIntersection(resources, ray);

        if( isinf(sample.length) ){
            accumulatedColor += skyColor * lightColor;
            break;
        }

        ray->origin = sample.point;

        struct Material material =  materials[ sample.materialID ];
        struct Object object = objects[ sample.objectID ];
        struct Texture info = infos[ 0 ]; // Replace with acutal texture idx

        float3 normal = sample.normal;

        float3 lightVector = normalize(-ray->direction);
        float3 viewVector = normalize(camera->position - sample.point);
        float3 diffusionDirection = DiffuseReflect(normal, seed);
        float3 reflectionDirection = Reflect(ray->direction, normal);
        float3 refractionDirecton = Refract(ray->direction, normal, lastIOR, material.indexOfRefraction);

        float3 halfVector = normalize(normal + reflectionDirection);

        float cosLight = dot(normal, lightVector);

        float isMetallic = Rand(seed) <=  material.metallic;
        float isTransparent = Rand(seed) <=  material.transparency;

        float3 direction = mix(diffusionDirection, reflectionDirection, material.metallic * isMetallic);
        ray->direction = normalize(mix(direction, refractionDirecton, material.transparency * isTransparent));
        
        float4 emissionComponent =  material.albedo * material.emmissionIntensity * (cosLight > 0.0f);
        float4 pbrComponent = physicalBRDF(normal, halfVector, lightVector, ray->direction, viewVector, &material);

        float4 color = 0.0f;
        
        if ( material.textureID > -1 )
            color = GetTexturePixel(texture, &object, info, sample.point, sample.normal);

        accumulatedColor +=  ( emissionComponent + pbrComponent + color) * lightColor ;


        lightColor *= 2.0f * cosLight * material.albedo;
        lastIOR = material.indexOfRefraction;
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
    float pixelXPos = (2.0 * x / width - 1.0f) * camera->aspectRatio  ;
    float pixelYPos = (2.0 * y / height - 1.0f);

    return camera->position + (camera->front + ( camera->right * pixelXPos + camera->up * pixelYPos) * tanHalfFOV ) * camera->near;
}



// Main

void kernel RayTrace(
    write_only image2d_t image, 
    global struct Resources * resources, 
    const struct Camera camera, 
    const int numFrames,
    global float4 * scratch 
    ){

    local struct Resources localResources;
    localResources = *resources;

    uint x = get_global_id(0);
    uint y = get_global_id(1);

    uint width = get_global_size(0);
    uint height = get_global_size(1);

    uint index = y * width + x;
    uint seed = (numFrames<<16) ^ (numFrames >>13) + index;

    // Simple anti-aliasing techinque
    float3 offset = RandomDirection(&seed);
    float3 pixelPosition = CalculatePixelPosition(x + offset.x + 0.5f, y + offset.y + 0.5f, width, height, &camera);

    struct Ray ray;
    ray.origin = camera.position;
    ray.direction = normalize(pixelPosition - ray.origin);

    // Monte - Carlo path tracing have issue with glaring (dark and light spots on consistent color)

    // Supersampling approach results in no noise but very low amount of fps

    float4 sample = ComputeColor(localResources, &ray, &camera, &seed);

    float scale = 1.0f / (numFrames + 1);

    float4 pixel = mix(scratch[index], sample, scale);

    write_imagef(image, (int2)(x, y), pixel);

    scratch[ index ] = pixel;

}
