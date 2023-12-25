#ifndef CAMERA_H
#define CAMERA_H

#include "Vector3.h"

constexpr float deg2rad = 3.1415926535f/180.0f;
const Vector3 worldUp = {0.0f , 1.0f ,0.0f};

struct Camera{

    Vector3 front = {0.0f, 0.0f, 1.0f};
    Vector3 up = {0.0f, 1.0f, 0.0f};
    Vector3 right = {1.0f, 0.0f, 0.0f}; 

    Vector3 position = {0.0f, 0.0f, 0.0f};

    float movementSpeed = 100.0f;
    float rotationSpeed = 15.0f;
    float aspectRatio = 1.0f;
    float nearView = 0.1f;
    float farView = 100.0f;
    float fov = 45.0f;
    float pitch = 0.0f;
    float yaw = 90.0f;

    float lastMouseX;
    float lastMouseY;

    void Rotate(const float& currentX, const float& currentY){
        float offsetX = lastMouseX - currentX;
        float offsetY = lastMouseY - currentY;

        lastMouseX = currentX;
        lastMouseY = currentY;

        float sensitivity = 0.1f;
        offsetX *= sensitivity;
        offsetY *= sensitivity;

        yaw = yaw + offsetX;
        pitch = pitch + offsetY;

        pitch = std::fmax(-89.0f, std::fmin(pitch, 89.0f));

        float cosPitch = cos(pitch * deg2rad);

        front.x = cos(yaw * deg2rad) * cosPitch;
        front.y = sin(pitch * deg2rad);
        front.z = sin(yaw * deg2rad) * cosPitch;

        front = front.Normalize();

        right = Vector3::CrossProduct(worldUp, front);
        up  = Vector3::CrossProduct(front, right);

    }

    void Move(const Vector3& direction, const float& deltaTime){

        position = position + (Vector3)direction * movementSpeed * deltaTime;

    }

    Vector3 CalculatePixelPosition(const int x, const int y, const int width, const int height){
        float tanHalfFOV = tan(deg2rad * fov * 0.5f);
        float cameraX = (2.0 * x / width - 1.0f) * aspectRatio * tanHalfFOV * nearView;
        float cameraY = (2.0 * y / height - 1.0f) * tanHalfFOV * nearView;
        
        return position + front*nearView + right*cameraX + up*cameraY;

    }
};


#endif