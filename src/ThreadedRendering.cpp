#include "ThreadedRendering.h"

void ThreadedRendering::Init(RenderingContext * _context){
    this->context = _context;

    fprintf(stdout, "========[ CPU Config ]========\n");

    numThreads = std::thread::hardware_concurrency();
    fprintf(stdout, "Logic cores : %d\n", numThreads);

    threads = new std::thread[numThreads];

    rowsPerThread = context->height / numThreads;
}

Vector3 ThreadedRendering::RandomReflection(const struct Vector3& normal, unsigned int& seed){
    Vector3 direction;
    direction.x = Random::UniformRandom(seed);
    direction.y = Random::UniformRandom(seed);
    direction.z = Random::UniformRandom(seed);

    bool lessThanZero = Vector3::DotProduct(normal, direction) < 0;

    return (direction * ((lessThanZero * -1.0f) + (1-lessThanZero))).Normalize();
}

Vector3 ThreadedRendering::RandomDirection(unsigned int& seed){
    return (struct Vector3){
        Random::UniformRandom(seed),
        Random::UniformRandom(seed),
        Random::UniformRandom(seed)
    };
}

Vector3 ThreadedRendering::Reflect(const Vector3& incident, const Vector3& normal) {
    return incident - (Vector3)normal * (2.0f * Vector3::DotProduct(incident, normal)) ;
}

float ThreadedRendering::Intersect(const Ray &ray, const Sphere &sphere) {
    const Vector3 oc = ray.origin - sphere.position;

    const float b = Vector3::DotProduct(oc, ray.direction);
    const float c = Vector3::DotProduct(oc, oc) - sphere.radius * sphere.radius;
    const float delta = b * b - c;

    const float sqrtDelta = std::sqrt(delta);
    const float t1 = (-b - sqrtDelta);
    const float t2 = (-b + sqrtDelta);

    return fmin(t1, t2);
}

Color ThreadedRendering::ComputeColor(struct Ray& ray, unsigned int& seed) {
    Color accumulatedColor = {0};
    Color tempColor = {1.0f, 1.0f, 1.0f, 1.0f};

    for(int i = 0; i < 10; ++i){
        HitInfo info = FindClosestIntersection(ray);
        if(info.distance < INFINITY){
            ray.origin = info.point;

             Material * material = &info.material;

            Vector3 diffusionDir = (info.normal + RandomReflection(info.normal, seed)).Normalize();
            Vector3 specularDir = Reflect(ray.direction, info.normal);

            ray.direction = Vector3::Lerp(diffusionDir, specularDir, material->smoothness);

            struct Color emmisionComponent = Color::ToneColor(material->emission, material->emmissionScale);
            struct Color diffuseComponent = Color::ToneColor(material->diffuse, Vector3::DotProduct(info.normal, diffusionDir) * material->diffusionScale);

            accumulatedColor = accumulatedColor +  Color::MixColors(emmisionComponent, tempColor);
            accumulatedColor = accumulatedColor + Color::MixColors(diffuseComponent, tempColor);

            tempColor = Color::MixColors(tempColor, material->baseColor);

        }else{
            break;
        }
    }

    return accumulatedColor;
}


HitInfo ThreadedRendering::FindClosestIntersection(const Ray& ray){
    HitInfo info;
    info.distance = INFINITY;

    for (int i = 0; i < context->spheres.size(); i++) {
        float distance = Intersect(ray, context->spheres[i]);

        if(distance > 0.0f && distance < info.distance){
            info.distance = distance;
            info.point = ray.origin + (Vector3)ray.direction * distance;
            info.normal = (info.point -  context->spheres[i].position).Normalize();
            info.material = context->spheres[i].material;
        }
    }

    return info;
}

void ThreadedRendering::ComputeRows(int _startY, int _endY, Color* pixels) {

    for (int y = _startY; y < _endY; ++y) {
        for (int x = 0; x < context->width; ++x) {

            Vector3 pixelPosition = context->camera.CalculatePixelPosition(x, y, context->width, context->height);

            unsigned int index = y * context->width + x;
            unsigned int seed = context->frameCounter * 93726103484 + index;

            Ray ray;
            ray.origin = context->camera.position;
            ray.direction = (pixelPosition - ray.origin).Normalize();

            struct Color finalColor = ComputeColor(ray, seed);

            float scale = 1.0f / (context->frameCounter+1);

            pixels[index] =  Color::LerpColor(pixels[index], finalColor, scale);
        }
    }
}

void ThreadedRendering::Render(Color * array){

    for (int i = 0; i < numThreads; i++) {
        int startY = i * rowsPerThread;
        bool last = (i==numThreads-1);
        int endY =  (last) * context->height +  (1-last)*(startY + rowsPerThread);

        threads[i] = std::thread([this, startY, endY, array](){
                this->ComputeRows(startY, endY, array);
        });
    }

    for (int i = 0; i < numThreads; i++){
        threads[i].join();
    }

}


ThreadedRendering::~ThreadedRendering(){
    delete[] threads;
}

