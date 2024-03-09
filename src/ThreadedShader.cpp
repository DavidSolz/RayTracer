#include "ThreadedShader.h"


ThreadedShader::ThreadedShader(RenderingContext * _context) : ComputeShader(_context){

    numThreads = context->numThreads;

    context->loggingService.Write(MessageType::INFO, "Discovered %d logic cores", std::thread::hardware_concurrency());
    context->loggingService.Write(MessageType::INFO, "Using %d threads", numThreads);

    threads = new std::thread[numThreads];

    rowsPerThread = context->height / numThreads;

    if ( context->bvhAcceleration == true){
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

Color ThreadedShader::ComputeColor(struct Ray& ray, unsigned int& seed) {

    Color accumulatedColor = {0.0f, 0.0f, 0.0f, 0.0f};
    Color colorMask = {1.0f, 1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;

    for(int i = 0; i < 2; ++i){
        Sample sample = traverse(context, ray);

        if(sample.distance == INFINITY){
            //accumulatedColor += GetSkyBoxColor(intensity, ray);
            break;
        }
            ray.origin = sample.point;

        //     Material * material = &context->materials[sample.materialID];

        //     Vector3 diffusionDirection = RandomReflection(sample.normal, seed);
        //     Vector3 specularDirection = Reflect(ray.direction, sample.normal);

        //     ray.direction = Vector3::Lerp(diffusionDirection, specularDirection, material->metallic);

        //     float lightIntensity = Vector3::DotProduct(ray.direction, sample.normal);
        //     lightIntensity = fmax(0.0f, fmin(lightIntensity, 1.0f));

        //     Color emmisionComponent = material->albedo * material->emmissionIntensity;
        //     Color diffuseComponent = material->albedo * 2 * lightIntensity;

        //     accumulatedColor += (diffuseComponent + emmisionComponent) * colorMask;
        //     colorMask *= material->albedo;
        //     intensity *= lightIntensity * 0.1f;
    }

    return accumulatedColor;
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

            float scale = 1.0f / (context->frameCounter+1);

            pixels[index] =  pixels[index] +  (sample - pixels[index]) * scale;
        }
    }
}

void ThreadedShader::Render(Color * _pixels){
    for (int i = 0; i < numThreads; ++i) {
        int startY = i * rowsPerThread;
        int endY =  startY + rowsPerThread;

        threads[i] = std::thread(
        [this, startY, endY, _pixels](){
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

    struct Sample sample = {0};
    sample.distance = INFINITY;
    float length = -1.0f;

    Vector3 scaledDir = ray.direction * EPSILON;

    for (int id = 0; id < context->objects.size(); ++id) {

        struct Object object = context->objects[id];

        if ( object.type == TRIANGLE ){
                //length = IntersectTriangle(ray, &object);
            }else{
                //length = IntersectSphere(ray, &object);
            }
            
            if( (length < sample.distance) && (length > 0.01f) ){

                sample.distance = length ;
                sample.point = ray.origin + scaledDir * length ;
                sample.objectID = id;

                if ( object.type == TRIANGLE ){
                    sample.normal = object.normal.Normalize();
                }else{
                    sample.normal = (sample.point - object.position).Normalize();
                }
                
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

Sample ThreadedShader::BVHTraverse(RenderingContext * context, const Ray & ray){

    struct Sample sample = {0};
    sample.distance = INFINITY;
    float length = -1.0f;

    std::stack<int> stack;

    Vector3 scaledDir = ray.direction * EPSILON;

    stack.push(0);

    while ( !stack.empty() ) {
        
        int boxID = stack.top();
        stack.pop();
        struct BoundingBox box = context->boxes[ boxID ];

        int leftChildIndex = box.leftID;
        int rightChildIndex = box.rightID;

        if ( box.objectID != -1 ) {

            struct Object object = context->objects[ box.objectID ];

            if ( object.type == TRIANGLE ){
                //length = IntersectTriangle(ray, &object);
            }else{
                //length = IntersectSphere(ray, &object);
            }
            
            if( (length < sample.distance) && (length > 0.01f) ){

                sample.distance = length ;
                sample.point = ray.origin + scaledDir * length ;
                sample.objectID = box.objectID;

                if ( object.type == TRIANGLE ){
                    sample.normal = object.normal.Normalize();
                }else{
                    sample.normal = (sample.point - object.position).Normalize();
                }
                
            }

        } else {

            if( leftChildIndex > 0){
                BoundingBox & left = context->boxes[leftChildIndex];

                if( AABBIntersection(ray, left.minimalPosition, left.maximalPosition) )
                    stack.push(leftChildIndex);
            }
                
            if( rightChildIndex > 0){
                BoundingBox & right = context->boxes[rightChildIndex];

                if( AABBIntersection(ray, right.minimalPosition, right.maximalPosition) )
                    stack.push(rightChildIndex);
            }

        }
    }

    return sample;
}

ThreadedShader::~ThreadedShader(){
    delete[] threads;
}