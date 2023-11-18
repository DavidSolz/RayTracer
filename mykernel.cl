
struct Material {
    uchar4 ambient;
    uchar4 diffuse;
    uchar4 specular;
    uchar4 emission;
    float shininess;
    float diffuseLevel;
};

struct Vector3{
    float x, y, z;
};

struct Sphere{
    float radius;
    struct Vector3 position;
    struct Material material;
};

struct Vector3 Sub(const struct Vector3 a, const struct Vector3 b) {
    struct Vector3 result;
    result.x = a.x - b.x;
    result.y = a.y - b.y;
    result.z = a.z - b.z;
    return result;
}

struct Vector3 Add(const struct Vector3 a, const struct Vector3 b) {
    struct Vector3 result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

struct Vector3 Mult(const struct Vector3 a, const float scalar){
    struct Vector3 result;
    result.x = a.x *scalar;
    result.y = a.y *scalar;
    result.z = a.z *scalar;
    return result;
}

float Magnitude(const struct Vector3 a){
    return sqrt(a.x*a.x+a.y*a.y+a.z*a.z);
}

struct Vector3 Normalize(const struct Vector3 a){
    struct Vector3 result;
    float magnitude = Magnitude(a);
    result.x = a.x/magnitude;
    result.y = a.y/magnitude;
    result.z = a.z/magnitude;
    return result;
}

uchar4 MixColor(uchar4 a, uchar4 b){
    uchar4 result;
    result.x = min(a.x + b.x, 255);
    result.y = min(a.y + b.y, 255);
    result.z = min(a.z + b.z, 255);
    result.w = min(a.w + b.w, 255);
    return result;
}

uchar4 ScaleColor(uchar4 color, float factor) {
    return (uchar4)(
        min(color.x * factor, 255.0f),
        min(color.y * factor, 255.0f),
        min(color.z * factor, 255.0f),
        min(color.w * factor, 255.0f)
    );
}

float Dot(const struct Vector3 a, const struct Vector3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

float Intersect(struct Vector3 rayOrigin, struct Vector3 rayDirection, global const struct Sphere *sphere) {
    struct Vector3 oc = Sub(rayOrigin, sphere->position);

    float b = Dot(oc, rayDirection);
    float c = Dot(oc, oc) - sphere->radius * sphere->radius;
    float delta = b * b - c;

    float sqrtDelta =  sqrt(delta);
    float t1 = (-b - sqrtDelta);
    float t2 = (-b + sqrtDelta);

    return fmin(t1, t2);
}

struct Vector3 Reflect(struct Vector3 incident, struct Vector3 normal) {
    return Sub(incident, Mult(normal, (2.0f * Dot(incident, normal))));
}

uchar4 ComputeColor(struct Vector3 intersectionPoint, struct Vector3 cameraPosition, global const struct Sphere *object, struct Vector3 sunPosition) {
    struct Vector3 normal = Normalize(Sub(intersectionPoint, object->position));
    struct Vector3 viewDir = Normalize(Sub(cameraPosition, intersectionPoint));
    struct Vector3 lightDir = Normalize(Sub(sunPosition, intersectionPoint));
    struct Vector3 reflectDir = Reflect(Mult(lightDir, -1.0f), normal);

    float specular = fmax(0.0f, Dot(viewDir, reflectDir));
    float lightPower = fmax(0.0f, Dot(normal, lightDir));

    uchar4 diffuseComponent = ScaleColor(object->material.diffuse, lightPower);
    uchar4 specularComponent = ScaleColor(object->material.specular, pow(specular, object->material.shininess));

    uchar4 resultColor = MixColor(MixColor(object->material.ambient, diffuseComponent), specularComponent);

    return resultColor; 

}

void kernel RenderGraphics(global uchar4* pixels, global const struct Sphere* objects, global const int * objects_count, global const struct Vector3 * cameraPosition, global const struct Sphere * sun){
    int x = get_global_id(0);
    int y = get_global_id(1);

    int width = get_global_size(0);
    int height = get_global_size(1);

    float aspectRatio = width/(float)height;

    int index = y * width + x;

    struct Vector3 pixelPosition = {x, y, 0};

    struct Vector3 rayDirection = Mult(Sub(pixelPosition, *cameraPosition), aspectRatio);
    rayDirection = Normalize(rayDirection);

    uchar4 color = {0, 0, 0, 0};

    for (int i = 0; i < *objects_count; i++) {
        float distance = Intersect(*cameraPosition, rayDirection, objects + i );
        struct Vector3 intersectionPoint = Add(*cameraPosition, Mult(rayDirection, distance));

        if(distance > 0.0f){

            color = ComputeColor(intersectionPoint, *cameraPosition, objects + i, sun->position);
            break;
        }
    }

    pixels[index] = color;
}

