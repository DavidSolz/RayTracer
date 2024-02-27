
#include "resources/kernels/KernelStructs.h"
// Defines
#define STACK_SIZE 32

#define PI 3.1416926535f
#define ONE_OVER_PI 1.0f/PI
#define ONE_OVER_2_PI 1.0f/(2.0f * PI)
#define PI_OVER_TWO PI/2.0f;
#define TWO_PI 2.0f * PI
#define ONE_OVER_MAX_CHAR 1.0f/255.0f

#define EPSILON 1.0000001f

/*

================COLOR MANIPULATION================

*/

float4 Unpack(const unsigned int color){
    unsigned char * byte = (unsigned char *)&color;
    return (float4)(byte[0], byte[1], byte[2], byte[3]) * ONE_OVER_MAX_CHAR;
}

float4 ColorSample(
    global const unsigned int * texture, 
    const float u, 
    const float v, 
    const int width, 
    const int height, 
    const float offset
    ){

    int texelX = floor( u * (width - 1) );
    int texelY = floor( v * (height - 1) );

    int idx = offset + texelY * width + texelX;

    return Unpack(texture[ idx ]);
}

float4 GetTexturePixel(
    global const unsigned int * texture,
    const struct Object * object,
    const struct Texture info,
    const float3 localPoint,
    const float3 normal
    ){

    float u = 0.0f;
    float v = 0.0f;
    float w = 0.0f;

    float3 relativePos = localPoint - object->position;

    if( object->type == CUBE ){

        float3 boxSize = object->maxPos - object->position;
        relativePos /= boxSize;

        float conditionU = fabs(normal.x) == 1.0f;
        float conditionV = fabs(normal.y) == 1.0f;

        u = conditionU * relativePos.y + (1.0f - conditionU) * relativePos.x;
        v = ( conditionV + conditionU ) * relativePos.z + (1.0f - conditionV - conditionU) * relativePos.y;

    } else if ( object->type == DISK ){

        float theta = atan2(relativePos.z, relativePos.x);
        float radius = length( relativePos );

        u = (theta + M_PI_F) / TWO_PI;
        v = radius / object->radius;

    } else if ( object->type == PLANE){

        float width = object->maxPos.x;
        float height = object->maxPos.y;

        float3 minPos = (float3)(-width , -height, 0.0f) * 0.5f;
        float3 localPos = (relativePos - minPos);

        u = localPos.x / width;
        v = 0.5f + localPos.z / height;

    }else if ( object->type == SPHERE){
        float theta = atan2(normal.z, normal.x) + M_PI;
        float phi = acos(normal.y);

        u = theta * ONE_OVER_2_PI;
        v = phi * ONE_OVER_PI;
    }else if( object->type == TRIANGLE ){

        float3 A = object->verticeA;
        float3 B = object->verticeB;
        float3 C = object->verticeC;

        float area = fabs((B.x - A.x) * (C.y - A.y) - (B.y - A.y) * (C.x - A.x));
        float areaPBC = fabs((B.x - localPoint.x) * (C.y - localPoint.y) - (B.y - localPoint.y) * (C.x - localPoint.x));
        float areaPCA = fabs((C.x - localPoint.x) * (A.y - localPoint.y) - (C.y - localPoint.y) * (A.x - localPoint.x));
        float areaPAB = fabs((A.x - localPoint.x) * (B.y - localPoint.y) - (A.y - localPoint.y) * (B.x - localPoint.x));

        u = clamp(areaPBC / area, 0.0f, 1.0f);
        v = clamp(areaPCA / area, 0.0f, 1.0f);
        w = clamp(1.0f - u - v, 0.0f, 1.0f);

        float3 interpolated = u * A + v * B + w * C;
        
        u = interpolated.x;
        v = interpolated.z;

    }

    u = clamp(u, 0.0f, 1.0f);
    v = clamp(v, 0.0f, 1.0f);

    return ColorSample(texture, u, v, info.width, info.height, info.offset);
}


/*

================INTERSECTIONS================

*/

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

float FlatIntersection(const struct Ray * ray, const float3 position, const float3 normal){

    float d = dot(position, normal);
    float rayToPlane = dot(ray->origin, normal);

    return (rayToPlane - d) / dot(-ray->direction, normal);
}

float IntersectPlane(const struct Ray * ray, const struct Object * object) {

    float length = FlatIntersection(ray, object->position, object->normal);
    float3 intersection = ray->origin + ray->direction * length;

    float halfWidth = object->maxPos.x * 0.5f;
    float halfHeight = object->maxPos.y * 0.5f;

    float condition = intersection.x < (object->position.x - halfWidth) ||
        intersection.x > (object->position.x + halfWidth) ||
        intersection.z < (object->position.z - halfHeight) ||
        intersection.z > (object->position.z + halfHeight);

    return (1.0f - condition) * length;
}

float IntersectDisk(const struct Ray * ray, const struct Object * object) {

    float length = FlatIntersection(ray, object->position, object->normal);
    float3 intersection = ray->origin + ray->direction * length * EPSILON;

    float3 delta = intersection - object->position;
    float distance = dot(delta, delta);
    float condition = distance <= (object->radius * object->radius);

    return condition * length;

}

float IntersectCube(const struct Ray * ray, const float3 minimalPosition , const float3 maximalPosition) {

    float3 dirs = sign(ray->direction);
    float3 values = fabs(ray->direction);

    const float3 minimalBias = 1e-6f;

    values = fmax(values, minimalBias);

    float3 direction = 1.0f/(values * dirs);

    float tMin = (minimalPosition.x - ray->origin.x) * direction.x;
    float tMax = (maximalPosition.x - ray->origin.x) * direction.x;

    float min = fmin(tMin, tMax);
    float max = fmax(tMin, tMax);

    tMin = min;
    tMax = max;

    float tyMin = (minimalPosition.y - ray->origin.y) * direction.y;
    float tyMax = (maximalPosition.y - ray->origin.y) * direction.y;

    min = fmin(tyMin, tyMax);
    max = fmax(tyMin, tyMax);

    tyMin = min;
    tyMax = max;

    if ( isgreater(tMin, tyMax) || isgreater(tyMin, tMax)) {
        return -1.0f;
    }

    tMin = fmax(tMin, tyMin);
    tMax = fmin(tMax, tyMax);

    float tzMin = (minimalPosition.z - ray->origin.z) * direction.z;
    float tzMax = (maximalPosition.z - ray->origin.z) * direction.z;

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

bool AABBIntersection(const struct Ray * ray, const float3 minimalPosition , const float3 maximalPosition){

    float3 invDirection = 1.0f / ray->direction;

    float3 tMin = (minimalPosition - ray->origin) * invDirection;
    float3 tMax = (maximalPosition - ray->origin) * invDirection;

    float3 t1 = fmin(tMin, tMax);
    float3 t2 = fmax(tMin, tMax);

    float tNear = fmax(t1.x, fmax(t1.y, t1.z));
    float tFar = fmin(t2.x, fmin(t2.y, t2.z));

    return tNear <= tFar && tFar > 0.0f;
}

float IntersectTriangle(const struct Ray * ray,const struct Object * object) {

    float3 A = object->verticeA;
    float3 B = object->verticeB;
    float3 C = object->verticeC;

    float3 e1 = (B - A);
    float3 e2 = (C - A);

    float3 normal = cross(ray->direction, e2);

    float determinant = dot(e1, normal);

    if( fabs(determinant) < 1e-6f)
        return -1.0f;

    float inverseDeterminant = 1.0f/determinant;
    float3 rayToTriangle = ray->origin - A;
    float u = inverseDeterminant * dot(rayToTriangle, normal);

    if( u < 0.0f || u >1.0f)
        return -1.0f;

    float3 q = cross(rayToTriangle, e1);
    float v = inverseDeterminant * dot(ray->direction, q);

    if( v < 0.0f || u+v >1.0f)
        return -1.0f;

    return inverseDeterminant * dot(e2, q);
}

void ComputeBoxNormal(struct Sample * sample,const struct Object * object){

    float3 boxCenter = (object->maxPos + object->position) * 0.5f;
    float3 radius = (object->maxPos - object->position) * 0.5f;
    float3 pointToCenter = sample->point - boxCenter;

    sample->normal = normalize(sign(pointToCenter) * step(fabs(fabs(pointToCenter) - radius), EPSILON));
}

struct Sample FindClosestIntersection(const struct Resources resources, const struct Ray * ray){

    global const struct BoundingBox * boxes = resources.boxes;

    global const struct Object * objects = resources.objects;
    int numObject = resources.numObject;

    struct Sample sample = {0};
    sample.length = INFINITY;

    int stack[STACK_SIZE];
    int top = 0;

    float3 scaledDir = ray->direction * EPSILON;

    stack[top++] = 0;

    while (top > 0) {
        
        int boxID = stack[--top];
        struct BoundingBox box = boxes[boxID];

        int leftChildIndex = box.leftID;
        int rightChildIndex = box.rightID;

        if ( box.objectID != -1 ) {

            struct Object object = objects[ box.objectID ];

            float length = -1.0f;

            switch(object.type){
                case CUBE:
                    length = IntersectCube(ray, object.position, object.maxPos);
                    break;
                case PLANE:
                    length = IntersectPlane(ray, &object);
                    break;
                case DISK:
                    length = IntersectDisk(ray, &object);
                    break;
                case TRIANGLE:
                    length = IntersectTriangle(ray, &object);
                    break;
                default:
                    length = IntersectSphere(ray, &object);
            }
            
            if( (length < sample.length) && (length > 0.01f) ){

                sample.length = length ;
                sample.point = ray->origin + scaledDir * length ;
                sample.objectID = box.objectID;

                switch(object.type){

                case CUBE:
                    ComputeBoxNormal(&sample, &object);
                    break;

                case SPHERE:
                    sample.normal = normalize(sample.point - object.position);
                    break;

                default :
                    sample.normal = normalize(object.normal);
                    break;
                }
            }

        } else {

            if( leftChildIndex > 0){
                struct BoundingBox left = boxes[leftChildIndex];

                if( AABBIntersection(ray, left.minimalPosition, left.maximalPosition) )
                    stack[top++] = leftChildIndex;
            }
                
            if( rightChildIndex > 0){
                struct BoundingBox right = boxes[rightChildIndex];

                if( AABBIntersection(ray, right.minimalPosition, right.maximalPosition) )
                    stack[top++] = rightChildIndex;
            }

        }
    }

    return sample;
}

float4 GetColorAtPoint(
    const struct Resources resources, 
    struct Ray * ray, 
    const struct Camera * camera
    ){

    global const unsigned int * textureData = resources.textureData;
    global const struct Material * materials = resources.materials;
    global const struct Object * objects = resources.objects;
    global const struct Texture * infos = resources.textureInfo;

    const float4 skyColor = (float4)(0.2f, 0.2f, 0.25f, 1.0f);

    struct Sample sample = FindClosestIntersection(resources, ray);

    if( isinf(sample.length) )
        return skyColor;

    struct Object object = objects[ sample.objectID ];
    struct Material material =  materials[ object.materialID ];
    struct Texture info = infos[ material.textureID ];
    float4 color = material.albedo * GetTexturePixel(textureData, &object, info, sample.point, sample.normal);

    return color;
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

void kernel ApplyTexture(
    write_only image2d_t image, 
    global struct Resources * resources, 
    const struct Camera camera, 
    global float4 * scratch 
    ){

    local struct Resources localResources;
    localResources = *resources;

    uint x = get_global_id(0);
    uint y = get_global_id(1);

    uint width = get_global_size(0);
    uint height = get_global_size(1);

    uint index = y * width + x;

    float3 pixelPosition = CalculatePixelPosition(x, y, width, height, &camera);

    struct Ray ray;
    ray.origin = camera.position;
    ray.direction = normalize(pixelPosition - ray.origin);

    float4 lightMapSample = scratch[ index ];
    float4 sample = GetColorAtPoint(localResources, &ray, &camera);
    
    float4 pixel = lightMapSample * sample;

    write_imagef(image, (int2)(x, y), pixel);

}
