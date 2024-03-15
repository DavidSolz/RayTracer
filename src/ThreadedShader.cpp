#include "ThreadedShader.h"


ThreadedShader::ThreadedShader(RenderingContext * _context) : ComputeShader(_context){

    numThreads = context->numThreads;

    context->loggingService.Write(MessageType::INFO, "Discovered %d logic cores", std::thread::hardware_concurrency());
    context->loggingService.Write(MessageType::INFO, "Using %d threads", numThreads);

    threads = new std::thread[numThreads];

    rowsPerThread = context->height / numThreads;

    if ( context->bvhAcceleration == true && context->boxes.size() > 0){
        traverse = ThreadedShader::BVHTraverse;
    }else{
        traverse = ThreadedShader::LinearTraverse;
    }

}

Vector3 ThreadedShader::RandomDirection(unsigned int& seed){
    float latitude = acos(2.0f * Random::Rand(seed) - 1.0f) - 3.1415926353f/2.0f;
    float longitude = Random::Rand(seed) * 2 *  3.1415926535f;

    float cosLatitude = cos(latitude);

    return (Vector3){
        cosLatitude * cos(longitude),
        cosLatitude * sin(longitude),
        sin(latitude)
    };
}

Vector3 ThreadedShader::RandomReflection(const struct Vector3& normal, unsigned int& seed){
    Vector3 direction = RandomDirection(seed);
    float dotProduct = Vector3::DotProduct(normal, direction);
    int sign = 2*(dotProduct >= 0.0f)-1;

    return direction * sign;
}

Vector3 ThreadedShader::Reflect(const Vector3& incident, const Vector3& normal) {
    return incident - (Vector3)normal * (2.0f * Vector3::DotProduct(incident, normal)) ;
}

Color ThreadedShader::ComputeColor(struct Ray& ray, unsigned int& seed) {

    Color accumulatedLight = {0.0f, 0.0f, 0.0f, 0.0f};
    Color lightColor = {1.0f, 1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;

    for(int i = 0; i < 2; ++i){
        Sample sample = traverse(context, ray);

        if( sample.objectID < 0){
            //accumulatedColor += GetSkyBoxColor(intensity, ray);
            break;
        }
            ray.origin = sample.point;

        Object * object = &context->objects[sample.objectID];

        Material * material = &context->materials[object->materialID];

        Vector3 normal = object->normal;
        Vector3 lightVector = ray.direction;
        Vector3 diffusionDirection = RandomReflection(normal, seed);
        Vector3 reflectionDirection = Reflect(ray.direction, normal);

        ray.direction = Vector3::Lerp(diffusionDirection, reflectionDirection, material->metallic).Normalize();

        float lightIntensity = Vector3::DotProduct(ray.direction, normal);
        lightIntensity = fmax(0.0f, fmin(lightIntensity, 1.0f));

        float cosLight = -Vector3::DotProduct(normal, lightVector);

        Color emission = material->albedo * material->emmissionIntensity;
        Color diffuseComponent = material->albedo * 2 * lightIntensity;

        accumulatedLight += (emission + diffuseComponent) * lightColor; 
        lightColor *= material->albedo * 2.0f * cosLight; 
    }

    return accumulatedLight;
}

void ThreadedShader::ComputeRows(const int & _startY, const int & _endY, Color * pixels) {

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

            float scale = 1.0f / (context->frameCounter + 1);

            pixels[index] =  pixels[index] + ( sample - pixels[index] ) * scale;
        }
    }
}

void ThreadedShader::Render(Color * _pixels){
    for (int i = 0; i < numThreads; ++i) {
        int startY = i * rowsPerThread;
        int endY =  startY + rowsPerThread;

        threads[i] = std::thread(
        [this, startY, endY, &_pixels](){
            this->ComputeRows(startY, endY, _pixels);
        }
        );

    }

    ComputeRows((numThreads-1)*rowsPerThread, context->height, _pixels);

    for (int i = 0; i < numThreads; ++i){
        threads[i].join();
    }

}

Sample ThreadedShader::LinearTraverse(RenderingContext * context, const Ray & ray){

    struct Sample sample = {};
    float minLength = INFINITY;
    float length = -1.0f;
    float u,v;

    Vector3 scaledDir = ray.direction * EPSILON;

    for (int id = 0; id < context->objects.size(); ++id) {

        struct Object object = context->objects[id];

        if ( object.type == TRIANGLE ){
            length = IntersectTriangle(ray, object, u, v);
        }else{
            //length = IntersectSphere(ray, &object);
        }
            
        if( (length < minLength) && (length > 0.01f) ){

            minLength = length ;
            sample.point = ray.origin + scaledDir * length ;
            sample.objectID = id;
                
        }

    }

    return sample;
}

bool ThreadedShader::AABBIntersection(const Ray & ray, const Vector3 & minimalPosition , const Vector3 & maximalPosition){

    Vector3 invDirection = Vector3(1.0f / ray.direction.x, 1.0f / ray.direction.y, 1.0f/ ray.direction.z);

    Vector3 tMin = (minimalPosition - ray.origin) * invDirection;
    Vector3 tMax = (maximalPosition - ray.origin) * invDirection;

    Vector3 t1 = Vector3::Minimal(tMin, tMax);
    Vector3 t2 = Vector3::Maximal(tMin, tMax);

    float tNear = fmax(t1.x, fmax(t1.y, t1.z));
    float tFar = fmin(t2.x, fmin(t2.y, t2.z));

    return tNear <= tFar && tFar > 0.0f;
}

float ThreadedShader::IntersectTriangle(const Ray & ray, const Object & object, float & u, float & v){
    const float epsilon = 1e-6f;

    Vector3 A = object.verticeA;
    Vector3 B = object.verticeB;
    Vector3 C = object.verticeC;

    Vector3 e1 = (B - A);
    Vector3 e2 = (C - A);

    Vector3 normal = Vector3::CrossProduct(ray.direction, e2);
    float det = Vector3::DotProduct(e1, normal);

    if( fabs(det) < epsilon)
        return -1.0f;

    float f = 1.0f/det;
    Vector3 rayToTriangle = ray.origin - A;
    u = f * Vector3::DotProduct(rayToTriangle, normal);

    if( u < 0.0f || u >1.0f)
        return -1.0f;

    Vector3 q = Vector3::CrossProduct(rayToTriangle, e1);
    v = f * Vector3::DotProduct(ray.direction, q);

    if( v < 0.0f || u+v >1.0f)
        return -1.0f;

    return f * Vector3::DotProduct(e2, q);
}

Sample ThreadedShader::BVHTraverse(RenderingContext * context, const Ray & ray){

    struct Sample sample = {};
    float minLength = INFINITY;
    float length = -1.0f;
    float u, v;

    int stack[STACK_SIZE] = {};
    int size = 0;

    Vector3 scaledDir = ray.direction * EPSILON;

    stack[size++] = 0;

    while ( size > 0 )  {
        
        int boxID = stack[--size];

        BoundingBox box = context->boxes[ boxID ];

        int leftChildIndex = box.leftID;
        int rightChildIndex = box.rightID;

        if ( box.objectID != -1 ) {

            Object object = context->objects[ box.objectID ];

            if ( object.type == TRIANGLE ){
                length = IntersectTriangle(ray, object, u, v);
            }else{
                //length = IntersectSphere(ray, &object);
            }
            
            if( (length < minLength) && (length > 0.01f) ){

                minLength = length ;
                sample.point = ray.origin + scaledDir * length ;
                sample.objectID = box.objectID;
                
            }

        } else {

            if( leftChildIndex != -1 ){
                BoundingBox & left = context->boxes[leftChildIndex];

                if( AABBIntersection(ray, left.minimalPosition, left.maximalPosition) )
                    stack[size++] = leftChildIndex;
            }
                
            if( rightChildIndex != -1 ){
                BoundingBox & right = context->boxes[rightChildIndex];

                if( AABBIntersection(ray, right.minimalPosition, right.maximalPosition) )
                    stack[size++] = rightChildIndex;
            }

        }
    }

    return sample;
}

ThreadedShader::~ThreadedShader(){
    delete[] threads;
}