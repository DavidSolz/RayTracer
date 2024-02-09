#include "ThreadedRendering.h"

static RenderingContext *context;

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

ThreadedRendering::ThreadedRendering(RenderingContext * _context){
    context = _context;

    numThreads = std::thread::hardware_concurrency();
    bool isHyperThreadinEnabled = CheckForHyperthreading();

    char buffer[250]={0};

    context->loggingService.Write(MessageType::INFO, "Configuring CPU...");

    sprintf(buffer, "Discovered %d logic cores", numThreads);

    context->loggingService.Write(MessageType::INFO, buffer);

    sprintf(buffer, "Checking for hyperthreading : %s", isHyperThreadinEnabled ? "enabled":"disabled");

    context->loggingService.Write(MessageType::INFO, buffer);

    context->loggingService.Write(MessageType::INFO, "CPU configuration done");

    threads = new std::thread[numThreads];

    rowsPerThread = context->height / numThreads;
}

Vector3 ThreadedRendering::RandomReflection(const struct Vector3& normal, unsigned int& seed){
    Vector3 direction = RandomDirection(seed);
    float dotProduct = Vector3::DotProduct(normal, direction);
    int sign = 2*(dotProduct >= 0.0f)-1;

    return direction * sign;
}

Vector3 ThreadedRendering::RandomDirection(unsigned int& seed){
    float latitude = acos(2.0f * Random::Rand(seed) - 1.0f) - 3.1415926353f/2.0f;
    float longitude = Random::Rand(seed) * 2 *  3.1415926535f;

    float cosLatitude = cos(latitude);

    return (Vector3){
        cosLatitude * cos(longitude),
        cosLatitude * sin(longitude),
        sin(latitude)
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

float ThreadedRendering::IntersectPlane(const Ray & ray, const Object & object) {
    float d = Vector3::DotProduct(object.position, object.normal);
    float rayToPlane = Vector3::DotProduct(ray.origin, object.normal);
    float temp = -Vector3::DotProduct(ray.direction, object.normal);

    return (rayToPlane-d) / temp;
}

float ThreadedRendering::IntersectDisk(const Ray & ray, const Object & object) {

    float t = IntersectPlane(ray, object);

    Vector3 p = ray.origin + ray.direction * t * 1.000005f;
    Vector3 v = p - object.position;
    float d2 = Vector3::DotProduct(v, v);

    bool condition = (d2 <= object.radius * object.radius);

    return t * condition - 1 + condition;
}

float ThreadedRendering::IntersectTriangle(const Ray & ray, const Object & object){
    const float epsilon = 1e-6f;

    int idA = object.indicesID.x;
    int idB = object.indicesID.y;
    int idC = object.indicesID.z;

    Vector3 e1 = (context->mesh.vertices[idB] - context->mesh.vertices[idA]);
    Vector3 e2 = (context->mesh.vertices[idC] - context->mesh.vertices[idA]);

    Vector3 normal = Vector3::CrossProduct(ray.direction, e2);
    float det = Vector3::DotProduct(e1, normal);

    if( fabs(det) < epsilon)
        return -1.0f;

    float f = 1.0f/det;
    Vector3 rayToTriangle = ray.origin - context->mesh.vertices[idA];
    float u = f * Vector3::DotProduct(rayToTriangle, normal);

    if( u < 0.0f || u >1.0f)
        return -1.0f;

    Vector3 q = Vector3::CrossProduct(rayToTriangle, e1);
    float v = f * Vector3::DotProduct(ray.direction, q);

    if( v < 0.0f || u+v >1.0f)
        return -1.0f;

    return f * Vector3::DotProduct(e2, q);
}

float ThreadedRendering::IntersectCube(const Ray & ray, const Object & object) {

    Vector3 dirs = ((Vector3)ray.direction).Directions();
    Vector3 values = ((Vector3)ray.direction).Absolute();

    float minimalBias = 1e-6f;

    values.x = fmax(values.x, minimalBias);
    values.y = fmax(values.y, minimalBias);
    values.z = fmax(values.z, minimalBias);

    Vector3 direction = values * dirs;
    direction.x = 1.0f/direction.x;
    direction.y = 1.0f/direction.y;
    direction.z = 1.0f/direction.z;

    float tMin = (object.position.x - ray.origin.x) * direction.x;
    float tMax = (object.maxPos.x - ray.origin.x) * direction.x;

    float min = fmin(tMin, tMax);
    float max = fmax(tMin, tMax);

    tMin = min;
    tMax = max;

    float tyMin = (object.position.y - ray.origin.y) * direction.y;
    float tyMax = (object.maxPos.y - ray.origin.y) * direction.y;

    min = fmin(tyMin, tyMax);
    max = fmax(tyMin, tyMax);

    tyMin = min;
    tyMax = max;

    if ( (tMin > tyMax) || (tyMin > tMax)) {
        return -1.0f;
    }

    tMin = fmax(tMin, tyMin);
    tMax = fmin(tMax, tyMax);

    float tzMin = (object.position.z - ray.origin.z) * direction.z;
    float tzMax = (object.maxPos.z - ray.origin.z) * direction.z;

    min = fmin(tzMin, tzMax);
    max = fmax(tzMin, tzMax);

    tzMin = min;
    tzMax = max;

    if ( (tMin > tzMax) || (tzMin > tMax)) {
        return -1.0f;
    }

    tMin = fmax(tMin, tzMin);
    tMax = fmin(tMax, tzMax);

    if ( (tMin > 0.0f) ) {
        return tMin;
    }

    return -1.0f;
}

Vector3 ComputeBoxNormal(const Vector3 & nearVertice, const Vector3 & farVertice, const Vector3 & intersectionPoint){

    Vector3 boxCenter = (farVertice + nearVertice)*0.5f;
    Vector3 radius = (farVertice - nearVertice)*0.5f;
    Vector3 pointToCenter = intersectionPoint - boxCenter;

    static const float bias = 1.000001f;

    Vector3 sign = { std::copysign(1.0f, pointToCenter.x), std::copysign(1.0f, pointToCenter.y), std::copysign(1.0f, pointToCenter.z) };
    Vector3 abs = ((pointToCenter).Absolute() - radius).Absolute();

    Vector3 step = { (abs.x < bias) ? 1.0f : 0.0f, (abs.y < bias) ? 1.0f : 0.0f, (abs.z < bias) ? 1.0f : 0.0f };

    return sign * step;
}

Color ThreadedRendering::GetSkyBoxColor(const float & intensity, const Ray & ray){
    float t = intensity/fmax(ray.direction.y + 1.0f, 0.00001f);

    static const Color skyColor = (Color){ray.direction.x, ray.direction.y, ray.direction.z, 1.0f};

    return skyColor * t;
}

Color ThreadedRendering::ComputeColor(struct Ray& ray, unsigned int& seed) {

    Color accumulatedColor = {0.0f, 0.0f, 0.0f, 0.0f};
    Color colorMask = {1.0f, 1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;

    for(int i = 0; i < 8; ++i){
        Sample sample = FindClosestIntersection(ray);

        if(sample.distance == INFINITY){
            accumulatedColor += GetSkyBoxColor(intensity, ray);
            break;
        }
            ray.origin = sample.point;

            Material * material = &context->materials[sample.materialID];

            Vector3 diffusionDirection = RandomReflection(sample.normal, seed);
            Vector3 specularDirection = Reflect(ray.direction, sample.normal);

            ray.direction = Vector3::Lerp(diffusionDirection, specularDirection, material->metallic);

            float lightIntensity = Vector3::DotProduct(ray.direction, sample.normal);
            lightIntensity = fmax(0.0f, fmin(lightIntensity, 1.0f));

            Color emmisionComponent = material->emission * material->emmissionScale;
            Color diffuseComponent = material->baseColor *  material->diffusionScale * 2 * lightIntensity;

            accumulatedColor += (diffuseComponent + emmisionComponent) * colorMask;
            colorMask *= material->baseColor;
            intensity *= lightIntensity * 0.1f;
    }

    return accumulatedColor;
}

Sample ThreadedRendering::FindClosestIntersection(const Ray& ray){
    Sample sample = {0};
    sample.distance = INFINITY;

    for (int i = 0; i < context->objects.size(); i++) {
        float distance = -1.0f;

        IntersectionFunction function = intersectionFunctions[ context->objects[i].type ];

        if(function==NULL)
            continue;

        distance = function(ray, context->objects[i]);

        if(distance < sample.distance && distance > 0.1f){
            sample.distance = distance;
            sample.point = ray.origin + (Vector3)ray.direction * distance;
            sample.materialID = context->objects[i].materialID;

            switch(context->objects[i].type){

                case CUBE:
                    sample.normal = ComputeBoxNormal(context->objects[i].position, context->objects[i].maxPos, sample.point);
                    break;

                case SPHERE:
                    sample.normal = (sample.point - context->objects[i].position).Normalize();
                    break;

                default:
                    sample.normal = context->objects[i].normal;
            }

        }
    }

    return sample;
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

            struct Color sample = ComputeColor(ray, seed);

            float scale = 1.0f / (context->frameCounter+1);

            pixels[index] =  pixels[index] +  (sample - pixels[index]) * scale;
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

