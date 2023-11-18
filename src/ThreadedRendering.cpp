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

    const float a = 1.0f; //squared rayDirection (is normalized) length so we don't need to apply it anywhere
    const float b = Vector3::DotProduct(oc, rayDirection); //Actually it should be 2*dp because dp returns only half of b
    const float c = Vector3::DotProduct(oc, oc) - sphere.radius * sphere.radius; // oc^2 -R^2 = (oc-r)(oc+r)
    const float delta = b * b - c;

    const float sqrtDelta = std::sqrt(delta);
    const float t1 = (-b - sqrtDelta);
    const float t2 = (-b + sqrtDelta);

    return fmin(t1, t2);
}

Color ThreadedRendering::ComputeColor(const Vector3& intersectionPoint, const Vector3& cameraPos,const Sphere &sphere, const Vector3& sunPosition) {
    
    Vector3 normal = (intersectionPoint - sphere.position).Normalize();  // Normal vector
    Vector3 lightDir = (sunPosition - intersectionPoint).Normalize();
    Vector3 viewDir = (cameraPos - intersectionPoint).Normalize();

    // Lambertian reflection model (Diffuse reflection)
    float lightPower = std::fmax(0.0f, Vector3::DotProduct(normal, lightDir));
    Color diffuseComponent = sphere.material.diffuse * lightPower;

    // Specular reflection (Phong model)
    Vector3 reflectDir = Reflect(lightDir*-1.0f, normal).Normalize();
    float specular = std::pow(std::fmax(0.0f, Vector3::DotProduct(viewDir, reflectDir)), sphere.material.shininess);
    Color specularComponent = sphere.material.specular * specular;

    return (Color)sphere.material.ambient + diffuseComponent + specularComponent;
}

void ThreadedRendering::ComputeRows(int _startY, int _endY, Color* pixels) {

    for (int y = _startY; y < _endY; ++y) {
        for (int x = 0; x < context->width; ++x) {
            
            Vector3 rayDir = (Vector3(x , y , 0) - context->cameraPos).Normalize();

            Color finalColor = {0};

            for (int i = 0; i < context->spheres.size(); i++) {
                float distanceToObject = Intersect(context->cameraPos, rayDir, context->spheres[i]);
                if (distanceToObject > 0.0f) {
                    Vector3 intersectionPoint = context->cameraPos + rayDir * distanceToObject;
                    
                    Vector3 pointToSun = context->sun.position - intersectionPoint;
                    float distanceToSun = pointToSun.Magnitude();

                    Vector3 lightDir = pointToSun.Normalize();

                    bool intersect = false ;
                    for (int j = 0; j < context->spheres.size(); j++) {
                        if(i==j)continue;

                        float distanceToIntersection = Intersect(context->cameraPos, rayDir, context->spheres[i]);

                        if(distanceToIntersection > 0.0f && distanceToIntersection < distanceToSun){
                            intersect=true;
                            break;
                        }
                    }

                    if(intersect){
                        finalColor = context->spheres[i].material.ambient ;
                    }else{
                        finalColor = ComputeColor(intersectionPoint, context->cameraPos, context->spheres[i], context->sun.position);
                    }

                    break;
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
    delete threads;
}

