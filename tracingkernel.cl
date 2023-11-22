//Structures

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

struct Ray{
    struct Vector3 origin;
    struct Vector3 direction;
};

struct Sphere{
    float radius;
    struct Vector3 position;
    struct Material material;
};

struct HitInfo{
    bool didHit;
    float distance;
    struct Vector3 point;
    struct Vector3 normal;
    struct Material material;
};

//Functions

struct Vector3 Sub(const struct Vector3 vectorA, const struct Vector3 vectorB) {
    return (struct Vector3){
        vectorA.x - vectorB.x,
        vectorA.y - vectorB.y,
        vectorA.z - vectorB.z
    };
}

struct Vector3 Add(const struct Vector3 vectorA, const struct Vector3 vectorB) {
    return (struct Vector3){
        vectorA.x + vectorB.x,
        vectorA.y + vectorB.y,
        vectorA.z + vectorB.z
    };
}

struct Vector3 Mult(const struct Vector3 vectorA, const float scalar){
    return (struct Vector3){
        vectorA.x * scalar,
        vectorA.y * scalar,
        vectorA.z * scalar
    };
}

float Rand(unsigned int * seed){
    *seed = *seed * 747796405 + 2891336453;
    unsigned int result = ((*seed >>((*seed>>28u)+4u)) ^ *seed) * 277803737;
    result = (result>>22)^result;
    return result/4294967295.0f;
}

float Random(unsigned int * seed){
    float theta = 2 * 3.1415926f * Rand(seed);
    float rho = sqrt(-2 * log(Rand(seed)));
    return rho * cos(theta);
}

float Magnitude(const struct Vector3 a){
    return sqrt(a.x*a.x+a.y*a.y+a.z*a.z);
}

float Dot(const struct Vector3 a, const struct Vector3 b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

struct Vector3 Normalize(const struct Vector3 a){
    float magnitude = Magnitude(a);
    return (struct Vector3){
        a.x/magnitude,
        a.y/magnitude,
        a.z/magnitude
    };
}

struct Vector3 RandomReflection(const struct Vector3 normal, unsigned int *seed){
    struct Vector3 direction;
    direction.x = Random(seed);
    direction.y = Random(seed);
    direction.z = Random(seed);

    direction = Normalize(direction);

    return Mult(direction, sign(Dot(normal, direction)));
}

struct Vector3 RandomDirection(unsigned int *seed){
    return (struct Vector3){
        Random(seed),
        Random(seed),
        Random(seed)
    };
}

struct Color AddColors(struct Color colorA, struct Color colorB){
    return (struct Color){
        colorA.R + colorB.R,
        colorA.G + colorB.G,
        colorA.B + colorB.B,
        colorA.A + colorB.A
    };
}

struct Color BalanceColor(struct Color colorA, struct Color colorB, float t){
    t = fmax(0.0f, fmin(t, 1.0f));
    return (struct Color){
        (1-t)*colorA.R + t*colorB.R,
        (1-t)*colorA.G + t*colorB.G,
        (1-t)*colorA.B + t*colorB.B,
        (1-t)*colorA.A + t*colorB.A
    };
}

struct Color LerpColor(struct Color colorA, struct Color colorB, float t){
    t = fmax(0.0f, fmin(t, 1.0f));
    return (struct Color){
        colorA.R + (colorB.R - colorA.R) * t,
        colorA.G + (colorB.R - colorA.G) * t,
        colorA.B + (colorB.R - colorA.B) * t,
        colorA.A + (colorB.R - colorA.A) * t
    };
}

struct Color MixColors(struct Color colorA, struct Color colorB) {
    return (struct Color){
        colorA.R * colorB.R,
        colorA.G * colorB.G,
        colorA.B * colorB.B,
        colorA.A * colorB.A
    };
}

struct Color ToneColor(struct Color color, float factor) {
    factor = fmax(0.0f, fmin(factor, 1.0f));
    return (struct Color){
        color.R * factor,
        color.G * factor,
        color.B * factor,
        color.A * factor
    };
}

struct Color MaxColor(struct Color colorA, struct Color colorB){

    bool greater =  colorA.R > colorB.R &&
                    colorA.G > colorB.G &&
                    colorA.B > colorB.B ;

    return AddColors(ToneColor(colorA, greater), ToneColor(colorB, (1-greater)));
}

float Intersect(const struct Ray *ray, global const struct Sphere *sphere) {
    struct Vector3 originToSphereCenter = Sub(ray->origin, sphere->position);

    float b = Dot(originToSphereCenter, ray->direction);
    float c = Dot(originToSphereCenter, originToSphereCenter) - sphere->radius * sphere->radius;
    float delta = b * b - c;

    float sqrtDelta =  sqrt(delta);
    float t1 = (-b - sqrtDelta);
    float t2 = (-b + sqrtDelta);

    return fmin(t1, t2);
}

struct Vector3 LerpPoint(struct Vector3 begin, struct Vector3 end, float t){
    t = fmax(0.0f, fmin(t, 1.0f));
    return Add(begin, Mult(Sub(end,begin), t));
}

struct Vector3 Reflect(struct Vector3 incident, struct Vector3 normal) {
    return Sub(incident, Mult(normal, (2.0f * Dot(incident, normal))));
}

struct HitInfo FindClosestIntersection(global const struct Sphere* objects, global const int * objects_count, const struct Ray * ray){
    struct HitInfo info;
    info.didHit = false;
    info.distance = INFINITY;

    for (int i = 0; i < *objects_count; i++) {
        float distance = Intersect(ray, objects + i );

        if(distance > 0.0f && distance < info.distance){
            info.didHit = true;
            info.distance = distance;
            info.point = Add(ray->origin, Mult(ray->direction, distance));
            info.normal = Normalize(Sub(info.point, objects[i].position));
            info.material = objects[i].material;
        }
    }

    return info;
}


struct Color ComputeColor(struct Ray *ray, global const struct Sphere* objects, global const int * objects_count, unsigned int *seed) {

    struct Color accumulatedColor = {0};
    struct Color tempColor = {1.0f, 1.0f, 1.0f, 1.0f};

    for(int i = 0; i < 10; ++i){
        struct HitInfo info = FindClosestIntersection(objects, objects_count, ray);
        if(info.didHit){
            ray->origin = info.point;

            struct Material * material = &info.material;

            struct Vector3 diffusionDir = Normalize(Add(info.normal, RandomReflection(info.normal, seed)));
            struct Vector3 specularDir = Reflect(ray->direction, info.normal);

            ray->direction = LerpPoint(diffusionDir, specularDir, material->smoothness);

            struct Color emmisionComponent = ToneColor(material->emission, material->emmissionScale);
            struct Color diffuseComponent = ToneColor(material->diffuse,  material->diffusionScale);

            accumulatedColor = AddColors(accumulatedColor, MixColors(emmisionComponent, tempColor));
            accumulatedColor = AddColors(accumulatedColor, MixColors(diffuseComponent, tempColor));

            tempColor = MixColors(tempColor, material->baseColor);

        }else{
            break;
        }
    }

    return accumulatedColor;
}

//Main

void kernel RenderGraphics(global struct Color* pixels,
global struct Sphere* objects,
global const int * objects_count,
global const struct Vector3 * cameraPosition,
global const int *numFrames){

    int x = get_global_id(0);
    int y = get_global_id(1);

    int width = get_global_size(0);
    int height = get_global_size(1);

    struct Vector3 pixelPosition = {x, y, 0};

    unsigned int index = y * width + x;
    unsigned int seed = *numFrames * 93726103484 + index;

    struct Ray ray;
    ray.origin = *cameraPosition;
    ray.direction = Normalize(Sub(pixelPosition, ray.origin ));

    struct Color finalColor = ComputeColor(&ray, objects, objects_count, &seed);

    float scale = 1.0f / (*numFrames+1);

    pixels[index] =  BalanceColor(pixels[index], finalColor, scale);
}

