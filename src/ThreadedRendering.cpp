#include "ThreadedRendering.h"

void ThreadedRendering::Init(RenderingContext * _context){
    this->context = _context;

    fprintf(stdout, "========[ CPU Config ]========\n");

    numThreads = std::thread::hardware_concurrency();
    fprintf(stdout, "Logic cores : %d\n", numThreads);

    threads = new std::thread[numThreads];

    rowsPerThread = context->height / numThreads;
}

Vector3 ThreadedRendering::Reflect(const Vector3& incident, const Vector3& normal) {
    return incident - (Vector3)normal * (2.0f * Vector3::DotProduct(incident, normal)) ;
}

float ThreadedRendering::Intersect(const Vector3& rayOrigin, const Vector3& rayDirection, const Sphere& sphere) {
    const Vector3 oc = rayOrigin - sphere.position;

    const float a = 1.0f;
    const float b = Vector3::DotProduct(oc, rayDirection);
    const float c = Vector3::DotProduct(oc, oc) - sphere.radius * sphere.radius;
    const float delta = b * b - c;

    const float sqrtDelta = std::sqrt(delta);
    const float t1 = (-b - sqrtDelta);
    const float t2 = (-b + sqrtDelta);

    return fmin(t1, t2);
}

Color ThreadedRendering::ComputeColor(const Vector3& intersectionPoint, const Vector3& cameraPos,const Sphere &sphere, const Vector3& sunPosition) {

    Vector3 normal = (intersectionPoint - sphere.position).Normalize();
    Vector3 lightDir = (sunPosition - intersectionPoint).Normalize();
    Vector3 viewDir = (cameraPos - intersectionPoint).Normalize();

    float lightPower = std::fmax(0.0f, Vector3::DotProduct(normal, lightDir));
    Color diffuseComponent = sphere.material.diffuse * sphere.material.diffusionScale;

    Vector3 reflectDir = Reflect(lightDir*-1.0f, normal);
    Color emmissionComponent = sphere.material.emission * sphere.material.emmissionScale;

    return (Color)sphere.material.baseColor + diffuseComponent + emmissionComponent;
}

void ThreadedRendering::ComputeRows(int _startY, int _endY, Color* pixels) {

    for (int y = _startY; y < _endY; ++y) {
        for (int x = 0; x < context->width; ++x) {

            Vector3 rayDir = (Vector3(x , y , 0) - context->camera.position).Normalize();

            Color finalColor = {0};

            float minDistance = INFINITY;

            for (int i = 0; i < context->spheres.size(); i++) {
                float distance = Intersect(context->camera.position, rayDir, context->spheres[i]);
                if (distance > 0.0f && distance < minDistance) {
                    minDistance = distance;
                    Vector3 intersectionPoint = context->camera.position + rayDir * distance;

                    finalColor = ComputeColor(intersectionPoint, context->camera.position, context->spheres[i], context->sun.position);

                }
            }

            pixels[y * context->width + x] = finalColor;
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
    if(threads) delete[] threads;
}

