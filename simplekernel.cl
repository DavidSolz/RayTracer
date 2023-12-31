// Structures

struct Material {
    float4 baseColor;
    float4 diffuse;
    float4 specular;
    float4 emission;
    float smoothness;
    float emmissionScale;
    float diffusionScale;
    float transparency;
    float refractiveIndex;
} ;

enum SpatialType{
    SPHERE,
    PLANE,
    DISK,
    CUBE,
    CYLINDER
};

struct Ray{
    float3 origin;
    float3 direction;
};

struct Object{
    enum SpatialType type;
    float radius;
    float3 position;
    float3 normal;
    float3 maxPos;
    unsigned int materialID;
};

struct Sample{
    float distance;
    float3 point;
    float3 normal;
    unsigned int materialID;
};

struct Camera{
    float3 front;
    float3 up;
    float3 right;

    float3 position;

    float movementSpeed;
    float aspectRatio;
    float near;
    float far;
    float fov;
    float pitch;
    float yaw;

};

// Functions

// PCG_Hash
float Rand(unsigned int * seed){
    *seed = *seed *747796405u + 2891336453u;
    unsigned int word = ((*seed >> ((*seed >> 28u) + 4u)) ^ *seed) * 277803737u;
    return ((word>>22u) ^ word)/(float)UINT_MAX;
}

float3 CalculatePixelPosition(const int x, const int y, const int width, const int height, global const struct Camera * camera){
    
    float tanHalfFOV = tan(radians(camera->fov) * 0.5f);
    float pixelXPos = (2.0 * x / width - 1.0f) * camera->aspectRatio * tanHalfFOV * camera->near;
    float pixelYPos = (2.0 * y / height - 1.0f) * tanHalfFOV * camera->near;
    
    return camera->position + camera->front * camera->near + camera->right * pixelXPos + camera->up * pixelYPos;
}

// Main

void kernel RenderGraphics(
global float4* pixels,
global struct Object * objects,
global const int * numObject,
global struct Material * materials,
global const struct Camera * camera,
global const int *numFrames
){

    int width = get_global_size(0);
    int height = get_global_size(1);

    int x = get_global_id(0);
    int y = get_global_id(1);

    if (x >= width || y >= height) {
        return;
    }

    float c_real = x * 3.0 / (width - 1) - 2.0;
    float c_imag = y * 2.0 / (height - 1) - 1.0;

    float z_real = 0.0;
    float z_imag = 0.0;
    float tmp_z_real;
    float norm;

    unsigned int divergence_at = 0;
    for (int i = 1; i <= 256; ++i) {
        tmp_z_real = z_real * z_real - z_imag * z_imag + c_real;
        z_imag = 2 * z_real * z_imag                   + c_imag;
        z_real = tmp_z_real;

        norm = z_real * z_real + z_imag * z_imag;
        if (norm > 4.0) {
            divergence_at = i;
            break;
        }
    }

    

    pixels[y * width + x] = (float4)(Rand(&divergence_at), Rand(&divergence_at),Rand(&divergence_at), 1.0f);
    
}

