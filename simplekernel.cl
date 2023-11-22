struct Color{
    float R, G, B, A;
};

struct Material {
    struct Color baseColor;
    struct Color diffuse;
    struct Color specular;
    struct Color emission;
    float smoothness;
    float emmissionScale;
    float diffusionScale;
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

struct Color MixColor(struct Color a, struct Color b){
    return (struct Color){
        a.R + b.R,
        a.G + b.G,
        a.B + b.B,
        a.A + b.A,
    };

}

struct Color ScaleColors(struct Color colorA, struct Color colorB) {
    return (struct Color){
        colorA.R * colorB.R,
        colorA.G * colorB.G,
        colorA.B * colorB.B,
        colorA.A * colorB.A
    };
}

struct Color ScaleColor(struct Color color, float factor) {
    factor = fmax(0.0f, fmin(factor, 1.0f));
    return (struct Color){
        color.R * factor,
        color.G * factor,
        color.B * factor,
        color.A * factor
    };
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

struct Color ComputeColor(struct Vector3 intersectionPoint, struct Vector3 cameraPosition, global const struct Sphere *object, struct Vector3 sunPosition) {
    struct Vector3 normal = Normalize(Sub(intersectionPoint, object->position));
    struct Vector3 viewDir = Normalize(Sub(cameraPosition, intersectionPoint));
    struct Vector3 lightDir = Normalize(Sub(sunPosition, intersectionPoint));
    struct Vector3 reflectDir = Reflect(Mult(lightDir, -1.0f), normal);

    float specular = fmax(0.0f, Dot(viewDir, reflectDir));
    float lightPower = fmax(0.0f, Dot(normal, lightDir));

    struct Color diffuseComponent = ScaleColor(object->material.diffuse, lightPower);
    struct Color specularComponent = ScaleColor(object->material.specular, pow(specular, object->material.emmissionScale));

    struct Color resultColor = MixColor(MixColor(object->material.baseColor, diffuseComponent), specularComponent);

    return resultColor;

}

void kernel RenderGraphics(global struct Color* pixels, global const struct Sphere* objects, global const int * objects_count, global const struct Vector3 * cameraPosition, global const struct Sphere * sun){
    int x = get_global_id(0);
    int y = get_global_id(1);

    int width = get_global_size(0);

    int index = y * width + x;

    struct Vector3 pixelPosition = {x, y, 0};

    struct Vector3 rayDirection = Normalize(Sub(pixelPosition, *cameraPosition));

    struct Color color = {0, 0, 0, 0};

    float minDistance = INFINITY;

    for (int i = 0; i < *objects_count; i++) {
        float distance = Intersect(*cameraPosition, rayDirection, objects + i );

        if(distance > 0.0f && distance < minDistance){
            minDistance = distance;
            struct Vector3 intersectionPoint = Add(*cameraPosition, Mult(rayDirection, minDistance));
            color = ComputeColor(intersectionPoint, *cameraPosition, objects + i, sun->position);
        }
    }

    pixels[index] = color;
}

