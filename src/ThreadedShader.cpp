#include "ThreadedShader.h"
#include "Shading.h"

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

    float latitude = acos(2.0f * Random::Rand(seed) - 1.0f) - PI_HALF;
    float longitude = Random::Rand(seed) * TWO_PI;

    float cosLatitude = cosf(latitude);

    return (Vector3){
        cosLatitude * cosf(longitude),
        cosLatitude * sinf(longitude),
        sinf(latitude)
    };
}

Vector3 ThreadedShader::DiffuseReflect(const struct Vector3& normal, unsigned int& seed){

    Vector3 randomDirection = RandomDirection(seed);
    float cosDirection = Vector3::DotProduct(normal, randomDirection);

    return (randomDirection * cosDirection + normal).Normalize();
}

Vector3 ThreadedShader::Reflect(const Vector3& incident, const Vector3& normal) {
    Vector3 outgoing = incident - (Vector3)normal * (2.0f * Vector3::DotProduct(incident, normal)) ;
    return outgoing.Normalize();
}

Vector3 ThreadedShader::Refract(const Vector3& incident, const Vector3& normal, const float & n1, const float & n2){

    float cosI = -Vector3::DotProduct(incident, normal);
    float sinR2 = (1.0f - cosI * cosI);

    float eta = n1/n2;

    if( eta * sinR2 > 1.0f)
        return {};//Reflect(incident, -normal);

    float cosR2 = sqrt(1.0f - sinR2 * sinR2);

    Vector3 direction = incident * eta + normal * (eta * cosI - cosR2);

    return direction.Normalize();
}

Vector3 ThreadedShader::CalculateWeights(const struct Material & material){
    float metallic = material.metallic;
    float transmission = (1.0f - material.metallic) * material.transparency;
    float dielectric = (1.0f - material.metallic) * (1.0f - material.transparency);

    Vector3 weights;

    weights.x = metallic + dielectric; //specular
    weights.y = transmission; // transmission
    weights.z = dielectric; // diffuse
    weights.w = material.clearcoatThickness; // clearcoat

    return weights.Normalize();
}

Color ThreadedShader::ComputeColor(Ray & ray, const Sample & sample, Color & lightSample, unsigned int& seed, const Vector3 & normal) {

    Object& object = context->objects[sample.objectID];
    Material& material = context->materials[object.materialID];
    Texture& info = context->textureInfo[ material.textureID ];

    Vector3 lightVector = (ray.direction * -1.0f).Normalize();
    Vector3 viewVector = (context->camera.position - sample.point).Normalize();
    Vector3 halfVector = (lightVector + viewVector).Normalize();

    Vector3 diffusionDirection = DiffuseReflect(normal, seed);
    Vector3 reflectionDirection = Reflect(ray.direction, normal);
    Vector3 refractionDirecton = Refract(viewVector, normal, INPUT_IOR, material.indexOfRefraction);

    Vector3 outgoing = Vector3::Lerp(diffusionDirection, reflectionDirection, material.metallic);

    ray.origin = sample.point;
    ray.direction = Vector3::Lerp(outgoing, refractionDirecton, material.transparency).Normalize();

    float cosLight = fmax(1e-6f, Vector3::DotProduct(normal, lightVector));
    float cosView = fmax(1e-6f, Vector3::DotProduct(normal, viewVector));
    float cosLightHalf = fmax(1e-6f, Vector3::DotProduct(lightVector, halfVector));

    Color emission = material.albedo * material.emmissionIntensity;
    float isEmissive = (emission.R + emission.G + emission.B) > 0.0f;

    Color texture = Shading::GetTexturePixel(context->textureData.data(), object, info, sample.point, normal);

    Color diffuseAlbedo = texture * material.tint * (1.0f - material.metallic);
    Color specularAlbedo = Color::Lerp(material.specular, WHITE, material.metallic);
    float fresnel = Shading::SchlickFresnel(cosLightHalf);

    Color diffuseComponent = diffuseAlbedo * (1.0f - fresnel) * Shading::DiffuseBRDF(cosView, cosLight, material);;
    Color specularComponent = specularAlbedo * fresnel * Shading::SpecularBSDF(normal, lightVector, viewVector, halfVector, material);
    Color transmissionComponent = Shading::SpecularTransmissionBSDF(lightVector, viewVector, halfVector, material);
    Color clearcoatComponent = Shading::ClearcoatBRDF(viewVector, lightVector, halfVector, material);
    Color sheenComponent = Shading::Sheen(cosLightHalf, material);

    Vector3 weights = CalculateWeights(material);

    Color colorSample = emission * isEmissive;
    colorSample = colorSample + (diffuseComponent + sheenComponent) * weights.z;
    colorSample = colorSample + clearcoatComponent * weights.w;
    colorSample = colorSample + specularComponent * weights.x;
    colorSample = colorSample + transmissionComponent * weights.y;
    colorSample = colorSample * lightSample * (cosLight > 0.0f);

    lightSample =  lightSample * texture * material.albedo * 2.0f * cosLight;

    return colorSample;
}

void ThreadedShader::ComputeRows(const int & _startY, const int & _endY, Color * pixels) {

    int32_t numElements = _endY - _startY;

    for (int y = _startY; y < _endY; ++y) {
        for (int x = 0; x < context->width; ++x) {

            unsigned int index = y * context->width + x;
            unsigned int seed = (context->frameCounter<<16) ^ (context->frameCounter >>13) + index;

            Vector3 offset = RandomDirection(seed);
            Vector3 pixelPosition = context->camera.CalculatePixelPosition(x + offset.x, y + offset.y, context->width, context->height);

            Ray ray;
            ray.origin = context->camera.position;
            ray.direction = (pixelPosition - ray.origin).Normalize();

            Color accumulator = {0.0f, 0.0f, 0.0f, 0.0f};
            Color lightSample = WHITE;

            #pragma unroll
            for(int iter = 0 ; iter < 4; iter++){

                Vector3 normal;
                Sample sample = traverse(context, ray, normal);

                if( sample.objectID < 0){

                    const Texture & info = context->textureInfo[1];

                    float u = ( atan2(ray.direction.x, ray.direction.z) + 3.1415926535f ) * ONE_OVER_PI;
                    float v = acos(-ray.direction.y) * ONE_OVER_PI;

                    Color texel = Shading::ColorSample(context->textureData.data(), u, v, info.width, info.height, info.offset);

                    accumulator = accumulator + texel * lightSample;
                    break;
                }

                Color colorSample = ComputeColor(ray, sample, lightSample, seed, normal);

                lightSample = Color::Clamp(lightSample);
                accumulator = Color::Clamp(accumulator + colorSample);
            }

            float scale = 1.0f / (context->frameCounter + 1);
            pixels[index] =  Color::Lerp(pixels[index], accumulator, scale);

        }
    }
}

void ThreadedShader::Render(Color * _pixels){

    int32_t start;
    int32_t end;

    for (int i = 0; i < numThreads; ++i) {

        start = i * rowsPerThread;
        end =  (i == numThreads-1 ) ? context->height : start + rowsPerThread;

        threads[i] = std::thread(
            [this, start, end, _pixels](){
                this->ComputeRows(start, end, _pixels);
            }
        );

    }

    for (int i = 0; i < numThreads; ++i)
        threads[i].join();

}

Sample ThreadedShader::LinearTraverse(RenderingContext * context, const Ray & ray, Vector3 & normal){

    struct Sample sample = {};
    sample.objectID = -1;
    float minLength = INFINITY;
    float length = -1.0f;
    float u,v;

    Vector3 scaledDir = ray.direction * EPSILON;

    for (int id = 0; id < context->objects.size(); ++id) {

        struct Object object = context->objects[id];

        if ( object.type == TRIANGLE ){
            length = IntersectTriangle(ray, object, u, v);
        }else{
            length = IntersectSphere(ray, object);
        }

        if( (length < minLength) && (length > 0.01f) ){

            minLength = length ;
            sample.point = ray.origin + scaledDir * length ;
            sample.objectID = id;

        }

    }

    if( sample.objectID == -1 )
        return sample;

    struct Object object = context->objects[ sample.objectID ];

    if ( object.type == SPHERE){

        normal = ( sample.point - object.position).Normalize();

    }else if( object.type == TRIANGLE ){

        Vector3 & A = object.vertices[0];
        Vector3 & B = object.vertices[1];
        Vector3 & C = object.vertices[2];

        Vector3 v0 = (B - A);
        Vector3 v1 = (C - A);
        Vector3 v2 = sample.point - A;

        float dot00 = Vector3::DotProduct(v0, v0);
        float dot01 = Vector3::DotProduct(v0, v1);
        float dot02 = Vector3::DotProduct(v0, v2);
        float dot11 = Vector3::DotProduct(v1, v1);
        float dot12 = Vector3::DotProduct(v1, v2);

        float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
        float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
        float v = (dot00 * dot12 - dot01 * dot02) * invDenom;
        float w = 1.0f - u - v;

        normal = (object.normals[0] * w + object.normals[1] * u + object.normals[2] * v).Normalize();
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

    const Vector3 & A = object.vertices[0];
    const Vector3 & B = object.vertices[1];
    const Vector3 & C = object.vertices[2];

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

    if( v < 0.0f || (u+v) > 1.0f)
        return -1.0f;

    return f * Vector3::DotProduct(e2, q);
}

float ThreadedShader::IntersectSphere(const Ray &ray, const Object &object) {
    const Vector3 oc = ray.origin - object.position;

    const float b = Vector3::DotProduct(oc, ray.direction);
    const float c = Vector3::DotProduct(oc, oc) - object.radius * object.radius;
    const float delta = b * b - c;

    const float sqrtDelta = std::sqrt(delta);
    const float t1 = (-b - sqrtDelta);
    const float t2 = (-b + sqrtDelta);

    return fmin(t1, t2);
}

Sample ThreadedShader::BVHTraverse(RenderingContext * context, const Ray & ray, Vector3 & normal){

    struct Sample sample = {};
    sample.objectID = -1;
    float minLength = INFINITY;
    float length = -1.0f;
    float u, v;

    int stack[STACK_SIZE] = {};
    int size = 0;


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
                length = IntersectSphere(ray, object);
            }

            if( (length < minLength) && (length > 0.01f) ){

                minLength = length ;
                sample.point = ray.origin + ray.direction * length ;
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

    if( sample.objectID  < 0 )
        return sample;

    struct Object & object = context->objects[ sample.objectID ];

    if ( object.type == SPHERE){

        normal = ( sample.point - object.position).Normalize();

    }else if( object.type == TRIANGLE ){

        Vector3 & A = object.vertices[0];
        Vector3 & B = object.vertices[1];
        Vector3 & C = object.vertices[2];

        Vector3 v0 = (B - A);
        Vector3 v1 = (C - A);
        Vector3 v2 = sample.point - A;

        float dot00 = Vector3::DotProduct(v0, v0);
        float dot01 = Vector3::DotProduct(v0, v1);
        float dot02 = Vector3::DotProduct(v0, v2);
        float dot11 = Vector3::DotProduct(v1, v1);
        float dot12 = Vector3::DotProduct(v1, v2);

        float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
        float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
        float v = (dot00 * dot12 - dot01 * dot02) * invDenom;
        float w = 1.0f - u - v;

        normal = (object.normals[0] * w + object.normals[1] * u + object.normals[2] * v).Normalize();
    }

    return sample;
}

ThreadedShader::~ThreadedShader(){
    delete[] threads;
}
