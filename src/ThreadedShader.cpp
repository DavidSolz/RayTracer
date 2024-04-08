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

float SchlickFresnel(const float & value){
    float temp = 1.0f - value;
    return temp * temp * temp * temp * temp;
}

Color Tint(const Color & albedo){
    float luminance = albedo.R * 0.3f + albedo.G * 0.6f + albedo.B;
    float condition = luminance > 0.0f;
    return Color::Lerp(WHITE, albedo * (1.0f/luminance) , condition);
}

Color Sheen(const float & cosLightHalf, const struct Material & material){
    Color tint = Tint(material.albedo);
    Color sheen = Color::Lerp(WHITE, tint, material.tintRoughness);
    return sheen * SchlickFresnel(cosLightHalf) * material.sheen;
}

float DiffuseBRDF(const float & cosView, const float & cosLight, const struct Material & material){

    float FL = SchlickFresnel(cosLight);
    float FV = SchlickFresnel(cosView);

    float R = 0.5f + 2.0f * cosLight * cosLight * material.roughness * material.roughness;
    float retro = R * ( FL + FV + FL * FV * (R - 1.0f) );

    return ONE_OVER_PI * ( (1.0f - 0.5f * FL) * (1.0f - 0.5f * FV) + retro );
}

float GgxAnisotropic(const Vector3 & halfVector, const float & ax, const float & ay){

    float dotHX2 = halfVector.x * halfVector.x;
    float dotHY2 = halfVector.z * halfVector.z;
    float cos2Theta = cos(halfVector.y) * cos(halfVector.y);
    float ax2 = ax * ax;
    float ay2 = ay * ay;

    float temp = (dotHX2 / ax2 + dotHY2 / ay2 + cos2Theta);

    return ONE_OVER_PI * 1.0f / (ax * ay * temp * temp);
}

float SeparableSmithGGXG1BSDF(const Vector3 & vector, const Vector3 & halfVector, const float & ax, const float & ay){

    float cos2Theta = halfVector.y * halfVector.y;
    float sin2Theta = 1.0f - cos2Theta;

    float tanTheta = sqrt( sin2Theta/cos2Theta );

    float cos2Phi = vector.x * vector.x;
    float sin2Phi = 1.0f - cos2Phi;

    float a = sqrt( cos2Phi * ax * ax + sin2Phi * ay * ay);
    float a2Tan2Theta = a * a * tanTheta * tanTheta;

    float lambda = 0.5f * (-1.0f + sqrt( 1.0f + a2Tan2Theta ));
    return 1.0f / (1.0f + lambda);

}

float SpecularBSDF(const Vector3 & normal, const Vector3 & lightVector, const Vector3 & viewVector, const Vector3 & halfVector, const struct Material & material){

    float aspect = sqrt(1.0f - 0.9f * material.anisotropy);

    float roughnessSqr = material.roughness * material.roughness;

    float ax = fmax(ALPHA_MIN, roughnessSqr / aspect);
    float ay = fmax(ALPHA_MIN, roughnessSqr * aspect);

    float cosLight = Vector3::DotProduct(normal, lightVector);
    float cosView = Vector3::DotProduct(normal, viewVector);

    float D = GgxAnisotropic(halfVector, ax, ay);
    float Gl = SeparableSmithGGXG1BSDF(lightVector, halfVector, ax, ay);
    float Gv = SeparableSmithGGXG1BSDF(viewVector, halfVector, ax, ay);

    return D * Gl * Gv / (4.0f * cosLight * cosView);
}

Color SpecularTransmissionBSDF(const Vector3 & lightVector, const Vector3 & viewVector, const Vector3 & halfVector, const struct Material & material){
    float aspect = sqrt(1.0f - 0.9f * material.anisotropy);

    float roughnessSqr = material.roughness * material.roughness;

    float ax = fmax(ALPHA_MIN, roughnessSqr / aspect);
    float ay = fmax(ALPHA_MIN, roughnessSqr * aspect);

    float cosViewHalf = Vector3::DotProduct(viewVector, halfVector);

    cosViewHalf *= halfVector.y;

    float eta = 1.0f / material.indexOfRefraction;

    float D = GgxAnisotropic(halfVector, ax, ay);
    float Gl = SeparableSmithGGXG1BSDF(lightVector, halfVector, ax, ay);
    float Gv = SeparableSmithGGXG1BSDF(viewVector, halfVector, ax, ay);
    float F = eta + (1.0f - eta) * SchlickFresnel(cosViewHalf);

    float result = D * F * Gl * Gv;

    return {result, result, result, result};
}

float GTR(const float & cosLightHalf, const float & alpha){

    if( alpha >= 1.0f)
        return ONE_OVER_PI;

    float alphaSqr = alpha * alpha;
    float decAlphaSqr = alphaSqr - 1.0f;

    return ONE_OVER_PI * decAlphaSqr/( log2(alphaSqr)*(1.0f + decAlphaSqr * cosLightHalf * cosLightHalf) );
}

float SeparableSmithGGXG1(const float & cosine, const float & alpha){
    float a2 = alpha * alpha;
    return 2.0f / (1.0f + sqrt(a2 + (1 - a2) * cosine * cosine));
}

Color ClearcoatBRDF(const Vector3 & viewVector, const Vector3 & lightVector, const Vector3 & halfVector, const struct Material & material) {

    float cosHalf = fabs(halfVector.y);
    float cosView = fabs(viewVector.y);
    float cosLight = fabs(lightVector.y);
    float cosLightHalf = Vector3::DotProduct(lightVector, halfVector);

    float scale = 0.1f + (0.001f - 0.1f) * material.clearcoatRoughness;

    float D = GTR(cosHalf, scale);
    float Gl = SeparableSmithGGXG1(cosLight, 0.25f);
    float Gv = SeparableSmithGGXG1(cosView, 0.25f);
    float F = 0.04f + 0.96f * SchlickFresnel(cosLightHalf);

    float result = 0.25f * D * Gl * Gv * F;

    return {result, result, result, result};
}

Color Unpack(const unsigned int & color){
    unsigned char * byte = (unsigned char *)&color;
    return (Color){(float)byte[0], (float)byte[1], (float)byte[2], (float)byte[3]} * ONE_OVER_UCHAR_MAX;
}

Color BilinearFilter(
    const unsigned int * texture,
    const float & u,
    const float & v,
    const int & width,
    const int & height,
    const int & offset
) {

    float texCoord[2] = {u * (width - 1), v * (height - 1)};
    float texel[2] = {floor(texCoord[0]), floor(texCoord[1])};
    float frac[2] = {texCoord[0] - texel[0], texCoord[1] - texel[1]};

    Color texelColor00 = Unpack(texture[offset + (int)texel[1] * width + (int)texel[0]]);
    Color texelColor10 = Unpack(texture[offset + (int)texel[1] * width + (int)texel[0] + 1]);
    Color texelColor01 = Unpack(texture[offset + ((int)texel[1] + 1) * width + (int)texel[0]]);
    Color texelColor11 = Unpack(texture[offset + ((int)texel[1] + 1) * width + (int)texel[0] + 1]);

    Color color =
        texelColor00 * (1.0f - frac[0]) * (1.0f - frac[1]) +
        texelColor10 * frac[0] * (1.0f - frac[1]) +
        texelColor01 * (1.0f - frac[0]) * frac[1] +
        texelColor11 * frac[0] * frac[1];

    return color;
}

Color ColorSample(
    const unsigned int * texture,
    const float & u,
    const float & v,
    const int & width,
    const int & height,
    const float & offset
    ){
    return BilinearFilter(texture, u, v, width, height, offset);
}

Color GetTexturePixel(
    const unsigned int * texture,
    const struct Object & object,
    const struct Texture & info,
    const Vector3 & localPoint,
    const Vector3 & normal
    ){

    Vector3 texCoord;

    if ( object.type == SPHERE){
        float theta = atan2(normal.z, normal.x) + 3.1415926535f;
        float phi = acos(normal.y);

        texCoord.x = theta;
        texCoord.y = phi;
        texCoord = texCoord * ONE_OVER_PI;

    }else if( object.type == TRIANGLE ){

        Vector3 A = object.vertices[0];
        Vector3 B = object.vertices[1];
        Vector3 C = object.vertices[2];

        float area = ((B.x - A.x) * (C.y - A.y) - (B.y - A.y) * (C.x - A.x)) * 0.5f;
        float areaPBC = ((B.x - localPoint.x) * (C.y - localPoint.y) - (B.y - localPoint.y) * (C.x - localPoint.x)) * 0.5f;
        float areaPCA = ((C.x - localPoint.x) * (A.y - localPoint.y) - (C.y - localPoint.y) * (A.x - localPoint.x)) * 0.5f;
        float areaPAB = ((A.x - localPoint.x) * (B.y - localPoint.y) - (A.y - localPoint.y) * (B.x - localPoint.x)) * 0.5f;

        texCoord.x = areaPBC / area;
        texCoord.y = areaPCA / area;
        texCoord.z = 1.0f - texCoord.x - texCoord.y;

        texCoord = Vector3::Clamp(texCoord);
    }

    return ColorSample(texture, texCoord.x, texCoord.y, info.width, info.height, info.offset);
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

    Color texture = GetTexturePixel(context->textureData.data(), object, info, sample.point, normal);

    Color diffuseAlbedo = texture * material.tint * (1.0f - material.metallic);
    Color specularAlbedo = Color::Lerp(material.specular, WHITE, material.metallic);
    float fresnel = SchlickFresnel(cosLightHalf);

    Color diffuseComponent = diffuseAlbedo * (1.0f - fresnel) * DiffuseBRDF(cosView, cosLight, material);;
    Color specularComponent = specularAlbedo * fresnel * SpecularBSDF(normal, lightVector, viewVector, halfVector, material);
    Color transmissionComponent = SpecularTransmissionBSDF(lightVector, viewVector, halfVector, material);
    Color clearcoatComponent = ClearcoatBRDF(viewVector, lightVector, halfVector, material);
    Color sheenComponent = Sheen(cosLightHalf, material);

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

                    Color texel = ColorSample(context->textureData.data(), u, v, info.width, info.height, info.offset);

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

    if( v < 0.0f || u+v >1.0f)
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
