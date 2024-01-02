#include "ThreadedRendering.h"

bool ThreadedRendering::CheckForHyperthreading(){

    FILE *fp;
    char var[5] = {0};
    int corecount = 0;

#ifdef _WIN32 

    fp = popen("wmic cpu get NumberOfCores", "r");

    while (fgets(var,sizeof(var),fp) != NULL) 
        sscanf(var,"%d",&corecount);

#elif __APPLE__

    fp = popen("sysctl -n hw.physicalcpu", "r");

    while (fgets(var,sizeof(var),fp) != NULL) 
        sscanf(var,"%d",&corecount);

#else

    fp = popen("lscpu | grep 'Core(s) per socket' | awk '{print $NF}'", "r");
    
    fscanf(fd, "%d", &corecount);

#endif

    if(fp)
        pclose(fp);

    return numThreads > corecount;

}

void ThreadedRendering::Init(RenderingContext * _context){
    this->context = _context;

    fprintf(stdout, "========[ CPU Config ]========\n");

    numThreads = std::thread::hardware_concurrency();
    fprintf(stdout, "\tLogic cores : %d\n", numThreads);

    bool isHyperThreadinEnabled = CheckForHyperthreading();

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

    for(int i = 0; i < 8; ++i){
        Sample sample = FindClosestIntersection(ray);

        if(sample.distance == INFINITY){
            accumulatedColor += (Color){0.5f, 0.7f, 1.0f, 1.0f} * (intensity/std::fmax(ray.direction.y+1.0f, 0.00001f));
            break;
        }
            ray.origin = sample.point;

            Material * material = &context->materials[sample.materialID];

            Vector3 diffusionDir = (sample.normal + RandomReflection(sample.normal, seed)).Normalize();
            Vector3 specularDir = Reflect(ray.direction, sample.normal);

            ray.direction = Vector3::Lerp(diffusionDir, specularDir, material->smoothness);

            Color emmisionComponent = material->emission * material->emmissionScale;
            Color diffuseComponent = material->diffuse *  material->diffusionScale;

            float lightIntensity = Vector3::DotProduct(ray.direction, sample.normal);

            accumulatedColor += (diffuseComponent + emmisionComponent) * colorMask;

            colorMask *= material->baseColor;
            intensity *= lightIntensity * 0.5f;
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

