#ifndef KERNELSTRUCTS_H
#define KERNELSTRUCTS_H

#include "src/Texture.h"

struct Material {
    float4 albedo;
    float4 tint;
    float4 specular;
    float4 transmissionFilter;
    float specularIntensity;
    float transparency;
    float indexOfRefraction;
    float roughness;
    float metallic;
    float tintWeight;
    float tintRoughness;
    float clearcoatThickness;
    float clearcoatRoughness;
    float emmissionIntensity;
    float anisotropy;
    float anisotropyRotation;
    int textureID;
} __attribute((aligned(128)));

enum SpatialType{
    SPHERE,
    PLANE,
    DISK,
    CUBE,
    TRIANGLE
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
    float3 indiceID;
    unsigned int materialID;
} __attribute((aligned(128)));

struct Sample{
    float length;
    float3 point;
    float3 normal;
    float4 texel;
    unsigned int objectID;
    unsigned int materialID;
};

struct Camera{
    float3 front;
    float3 up;
    float3 right;

    float3 position;

    float movementSpeed;
    float rotationSpeed;
    float aspectRatio;
    float near;
    float far;
    float fov;
    float pitch;
    float yaw;

};

struct Resources{
    global const struct Object * objects;
    global const struct Material * materials;
    global const float3 * vertices;
    global const struct Texture * textureInfo;
    global const int * textureData;
    global const int * normalMap;
    int numObject;
    int numMaterials;
} __attribute((aligned(64)));

#endif