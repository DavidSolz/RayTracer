#include "ThreadedRendering.h"

void ThreadedRendering::Init(RenderingContext * _context){
    this->context = _context;

    fprintf(stdout, "========[ CPU Config ]========\n");

    numThreads = std::thread::hardware_concurrency();
    fprintf(stdout, "\tLogic cores : %d\n", numThreads);

    bool isHyperThreadinEnabled = false;

#ifdef _WIN32

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    isHyperThreadinEnabled = (numThreads == sysInfo.dwNumberOfProcessors);
#else
    long numProcessors = sysconf(_SC_NPROCESSORS_ONLN);

    isHyperThreadinEnabled = numThreads == numProcessors;

#endif

    fprintf(stdout, "\tHyperthreading : %s\n", isHyperThreadinEnabled?"YES":"NO");


    threads = new std::thread[numThreads];

    rowsPerThread = context->height / numThreads;
}

Vector3 ThreadedRendering::RandomReflection(const struct Vector3& normal, unsigned int& seed){
    Vector3 direction = RandomDirection(seed);

    bool lessThanZero = Vector3::DotProduct(normal, direction) < 0;

    return (direction * ((lessThanZero * -1.0f) + (1.0f - lessThanZero))).Normalize();
}

Vector3 ThreadedRendering::RandomDirection(unsigned int& seed){
    return (Vector3){
        2*Random::UniformRandom(seed)-1.0f,
        2*Random::UniformRandom(seed)-1.0f,
        2*Random::UniformRandom(seed)-1.0f
    };
}

Vector3 ThreadedRendering::Reflect(const Vector3& incident, const Vector3& normal) {
    return incident - (Vector3)normal * (2.0f * Vector3::DotProduct(incident, normal)) ;
}

float ThreadedRendering::IntersectSphere(const Ray &ray, const Object &object) {
    const Vector3 oc = ray.origin - object.position;

    const float b = Vector3::DotProduct(oc, ray.direction);
    const float c = Vector3::DotProduct(oc, oc) - object.radius * object.radius;
    const float delta = b * b - c;

    const float sqrtDelta = std::sqrt(delta);
    const float t1 = (-b - sqrtDelta);
    const float t2 = (-b + sqrtDelta);

    return fmin(t1, t2);
}

float ThreadedRendering::IntersectPlane(const Ray &ray, const Object &object) {
    float factor = Vector3::DotProduct(ray.direction, object.normal);

    if( fabs(factor) > 0.0f){
        Vector3 originToPlaneCenter = object.position - ray.origin;
        return Vector3::DotProduct(originToPlaneCenter, object.normal)/factor;
    }
   
    return 0.0f;
}

Color ThreadedRendering::ComputeColor(struct Ray& ray, unsigned int& seed) {
    
    Color accumulatedColor = {0.0f, 0.0f, 0.0f, 0.0f};
    Color colorMask = {1.0f, 1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;

    for(int i = 0; i < 10; ++i){
        Sample info = FindClosestIntersection(ray);

        if(info.distance == INFINITY){
            accumulatedColor = accumulatedColor + (Color){0.6f, 0.7f, 0.9f, 1.0f} * intensity;
            break;
        }
            ray.origin = info.point;

            Material * material = &context->materials[info.materialID];

            Vector3 diffusionDir = (info.normal + RandomReflection(info.normal, seed)).Normalize();
            Vector3 specularDir = Reflect(ray.direction, info.normal);

            ray.direction = Vector3::Lerp(diffusionDir, specularDir, material->smoothness);

            Color emmisionComponent = material->emission * material->emmissionScale;
            Color diffuseComponent = material->diffuse *  material->diffusionScale;

            accumulatedColor = accumulatedColor + emmisionComponent * colorMask;
            accumulatedColor = accumulatedColor + diffuseComponent * colorMask;

            colorMask = colorMask * material->baseColor;
            intensity *= 0.1f;
    }

    return accumulatedColor;
}


Sample ThreadedRendering::FindClosestIntersection(const Ray& ray){
    Sample info = {0};
    info.distance = INFINITY;

    for (int i = 0; i < context->objects.size(); i++) {
        float distance = INFINITY;

        if(context->objects[i].type == SPHERE){
            distance = IntersectSphere(ray, context->objects[i]);
        }else{
            distance = IntersectPlane(ray, context->objects[i]);
        }

        if(distance < info.distance && distance > 0.000001f){
            info.distance = distance;
            info.point = ray.origin + (Vector3)ray.direction * distance;
            info.normal = (info.point -  context->objects[i].position).Normalize();
            info.materialID = context->objects[i].materialID;
        }
    }

    return info;
}

void ThreadedRendering::ComputeRows(const int& _startY, const int& _endY, Color* pixels) {

    for (int y = _startY; y < _endY; ++y) {
        for (int x = 0; x < context->width; ++x) {

            unsigned int index = y * context->width + x;
            unsigned int seed = (context->frameCounter<<16) ^ (context->frameCounter >>13) + index;

            Vector3 offset = RandomDirection(seed);

            Vector3 pixelPosition = context->camera.CalculatePixelPosition(x + offset.x - 0.5f, y + offset.y - 0.5f, context->width, context->height);
            
            Ray ray;
            ray.origin = context->camera.position;
            ray.direction = (pixelPosition - ray.origin).Normalize();

            struct Color finalColor = ComputeColor(ray, seed);

            float scale = 1.0f / (context->frameCounter+1);

            pixels[index] =  pixels[index] +  (finalColor - pixels[index]) * scale;
        }
    }
}

void ThreadedRendering::Render(Color * _pixels){

    for (int i = 0; i < numThreads; ++i) {
        int startY = i * rowsPerThread;
        int endY =  startY + rowsPerThread;

        threads[i] = std::thread([this, startY, endY, _pixels](){
                this->ComputeRows(startY, endY, _pixels);
        });
    }

    ComputeRows((numThreads-1)*rowsPerThread, context->height, _pixels);

    for (int i = 0; i < numThreads; ++i){
        threads[i].join();
    }

}


ThreadedRendering::~ThreadedRendering(){
    delete[] threads;
}

