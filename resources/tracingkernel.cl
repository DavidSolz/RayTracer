// Defines

#define ONE_OVER_PI 1.0f/M_PI_F
#define ONE_OVER_2_PI 1.0f/M_PI_2_F

// Structures

struct Material {
    float4 albedo;
    float4 tint;
    float4 specular;
    float4 transmissionFilter;
    float specularIntensity;
    float transparency;
    float indexOfRefraction;
    float roughness;
    float metallic;
    float sheen;
    float tintRoughness;
    float clearcoatThickness;
    float clearcoatRoughness;
    float emmissionIntensity;
    float anisotropy;
    float anisotropyRotation;
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

typedef struct {
    global const struct Object * objects;
    global const struct Material * materials;
    global const float3 * vertices;
    int numObject;
    int numMaterials;
} Resources;


// Functions

// PCG_Hash
float Rand(uint * seed){
    *seed = *seed *747796405u + 2891336453u;
    uint word = ((*seed >> ((*seed >> 28u) + 4u)) ^ *seed) * 277803737u;
    return ((word>>22u) ^ word)/(float)UINT_MAX;
}

float3 RandomDirection(uint *seed){
    float latitude = Rand(seed) * 2.0f * 3.1415926535f - 3.1415926535f;
    float longitude = Rand(seed) * 2.0f *  3.1415926535f;

    float cosLatitude = cos(latitude);

    return normalize((float3)(
        cosLatitude * cos(longitude),
        cosLatitude * sin(longitude),
        sin(latitude)
    ));
}

float IntersectSphere(const struct Ray * ray, global const struct Object * object) {
    float3 originToSphereCenter = ray->origin - object->position;

    float b = dot(originToSphereCenter, ray->direction);
    float c = dot(originToSphereCenter, originToSphereCenter) - object->radius * object->radius;
    float delta = b * b - c;

    float sqrtDelta =  sqrt(delta);
    float t1 = (-b - sqrtDelta);
    float t2 = (-b + sqrtDelta);

    return fmin(t1, t2);
}

float IntersectPlane(const struct Ray * ray, global const struct Object * object) {

    float d = dot(object->position, object->normal);
    float rayToPlane = dot(ray->origin, object->normal);

    return (rayToPlane - d) / dot(-ray->direction, object->normal);
}

float IntersectDisk(const struct Ray * ray, global const struct Object * object) {

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

float IntersectCube(const struct Ray * ray, global const struct Object * object) {

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
    global const struct Object * object,
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


struct Sample FindClosestIntersection(global const Resources * resources, const struct Ray * ray){

    global const struct Object * objects = resources->objects;
    global const float3 * vertices = resources->vertices;
    int numObject = resources->numObject;

    struct Sample sample = {0};
    sample.length = INFINITY;

    for (int i = 0; i < numObject; ++i) {

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

    sample.normal = normalize(sample.normal);

    return sample;
}

// Negation of Phong illumination reflection formula
float3 Reflect(const float3 incoming, const float3 normal) {

    float cosIncoming = dot(incoming, normal);
    float3 outgoing = incoming - normal * 2.0f * cosIncoming;

    return normalize(outgoing);
}

float3 DiffuseReflect(const float3 normal, uint * seed){
    float3 direction = RandomDirection(seed);
    return normalize(direction * dot(normal, direction) + normal);
}

// Snells law refraction
float3 Refract(const float3 incoming, float3 normal, float n1, float n2){

    float cosI = dot(incoming, normal);
    float eta;

    if(cosI < 0.0f){
        eta = n1/n2;
        //normal = -normal;
    }else{
        eta = n2/n1;
    }

    float coeff = 1.0f - eta * eta * (1.0f - cosI*cosI);

    if ( coeff < 0.0f)
        return Reflect(incoming, normal);

    float3 direction = eta * incoming - (eta * cosI + sqrt(coeff) ) * normal;

    return direction;

}

float3 ReflectAnisotropic(float3 incident, float3 normal, float anisotropy) {
    float3 reflection = Reflect(incident, normal);
    float3 tangent = normalize(cross(normal, (float3)(0.0f, 1.0f, 0.0f)));
    float3 bitangent = cross(normal, tangent);
    float3 anisotropicReflection = normalize(reflection + anisotropy * (tangent + bitangent));

    return anisotropicReflection;
}

float4 SchlickFresnel(const float cosTransmit, const float4 F0){
    return F0 + (1.0f - F0) * pow(1.0f - cosTransmit, 5.0f);
}

float GGX(const float cosH, const float roughness){
    float alpha = roughness * roughness;
    float denominator = (cosH * cosH * (alpha - 1.0f) + 1.0f);
    return alpha/(M_PI_F*denominator*denominator + 1e-6f);
}

float SchlickGGX(const float cosView, const float roughness){
    float localRoughness = roughness + 1.0f;
    float coeff = localRoughness * localRoughness * 0.125f;

    return cosView /( cosView *(1.0f - coeff) + coeff + 1e-6f);
}

float GeometricSmithShlickGGX(const float cosView, const float cosLight, const float roughness){
    float ggx1 = SchlickGGX(cosView, roughness);
    float ggx2 = SchlickGGX(cosLight, roughness);

    return ggx1*ggx2;
}

float4 physicalBRDF(const float3 normal, const float3 halfVector, const float3 lightVector, const float3 outgoing, const float3 viewVector, struct Material * material){

    float cosHalf = dot(normal, halfVector);
    float cosView = dot(normal, viewVector);
    float cosLight =  dot(normal, lightVector);
    float cosOutgoing = dot(normal, outgoing);

    float eta = material->indexOfRefraction;

    float rS = (cosLight - eta*cosView)/(cosLight + eta*cosView);
    float rP = (eta*cosLight - cosView)/(eta*cosLight + cosView);

    float4 R0 = (rS*rS + rP*rP)*0.5f;

    float D = GGX(cosHalf, material->roughness);
    float G = GeometricSmithShlickGGX(cosView, cosLight, material->roughness);
    float4 F = SchlickFresnel(cosHalf, R0) * material->albedo;

    float denominator = 4.0f * cosView * cosLight + 1e-6f; 

    float4 F0 = mix(material->tint, material->albedo, material->sheen);
    float4 sheen = mix(material->albedo, F0, material->tintRoughness) * (1.0f - F0);
    sheen *= (1.0f - G)*(1.0f-G);

    return clamp((D * F * G + sheen)/ denominator, 0.0f , 1.0f)  ;

}

float perlin(const float2 * P){
    return clamp(sin(dot(*P, (float2)(12.9898f, 78.233f))) * 43758.5453f, 1e-6f, 1.0f);
}

float4 ComputeColor(
    global const Resources * resources, 
    struct Ray * ray, 
    const struct Camera * camera, 
    uint * seed,
    const float2 * noiseUV
    ){

    float4 accumulatedColor = 0.0f;
    float4 lightColor = 1.0f;
    float lastIOR = 1.0f;

    const float4 skyColor = (float4)(0.2f, 0.2f, 0.25f, 1.0f);
    global const struct Material * materials = resources->materials;

    float3 noisePoint = perlin(noiseUV);


    for(int i = 0; i < 8; ++i){
        struct Sample sample = FindClosestIntersection(resources, ray);

        if( isinf(sample.length) ){
            accumulatedColor += skyColor * lightColor;
            break;
        }

        ray->origin = sample.point;

        struct Material material =  materials[ sample.materialID ];

        float3 normal = sample.normal;

        float3 lightVector = normalize(-ray->direction);
        float3 viewVector = normalize(camera->position - sample.point);
        float3 diffusionDirection = DiffuseReflect(normal, seed);
        float3 reflectionDirection = Reflect(ray->direction, normal);
        float3 refractionDirecton = Refract(ray->direction, normal, lastIOR, material.indexOfRefraction);
        float3 anisotropicReflection = ReflectAnisotropic(ray->direction, normal, material.anisotropyRotation);

        float3 halfVector = normalize(normal + reflectionDirection);

        float cosLight = dot(normal, lightVector);
        float cosView = dot(normal, viewVector);

        float3 direction = mix(diffusionDirection, reflectionDirection, material.metallic);
        direction = mix(direction, anisotropicReflection, material.anisotropy);
        ray->direction = normalize(mix(direction, refractionDirecton, material.transparency));

        float4 emissionComponent = material.albedo * material.emmissionIntensity * (cosLight > 0.0f);
        float4 pbrComponent = physicalBRDF(normal, halfVector, lightVector, ray->direction, viewVector, &material);
        float4 absorptionComponent = material.transmissionFilter * exp(-material.albedo * sample.length); 

        accumulatedColor += (emissionComponent + pbrComponent + absorptionComponent) * lightColor ;
        lightColor *=  2.0f * cosLight  * material.albedo;
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
    write_only image2d_t image, 
    global Resources * resources, 
    const struct Camera camera, 
    const int numFrames,
    global float4 * scratch 
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
    ray.direction = normalize(pixelPosition - ray.origin);

    // Monte - Carlo path tracing have issue with glaring (dark and light spots on consistent color)

    float2 noise = (float2)(x/(float)width, y/(float)height);

    // Supersampling approach results in no noise but very low amount of fps

    float4 sample = ComputeColor(resources, &ray, &camera, &seed, &noise);

    float scale = 1.0f / (numFrames + 1);

    float4 pixel = mix(scratch[index], sample, scale);

    write_imagef(image, (int2)(x, y), pixel);

    scratch[ index ] = pixel;

}

void kernel Transfer(
    global Resources * resources,
    global struct Object * objects,
    global struct Material * materials,
    global const float3 * vertices,
    const int numObject,
    const int numMaterials
    ){

    resources->objects = objects;
    resources->numObject = numObject;
    resources->materials = materials;
    resources->numMaterials = numMaterials;
    resources->vertices = vertices;
}

void kernel AntiAlias(
    write_only image2d_t image, 
    global float4 * scratch 
    ){

    int x = get_global_id(0);
    int y = get_global_id(1);

    int width = get_global_size(0);
    int height = get_global_size(1);

    float4 pixelValue = 0.0f;

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {

            int neighborX = clamp(x + i, 0, width - 1);
            int neighborY = clamp(y + j, 0, height - 1);

            int index = neighborY * width + neighborX;

            pixelValue += scratch[index] ;

        }
    }


    write_imagef(image, (int2)(x,y), pixelValue/4.0f);

}
