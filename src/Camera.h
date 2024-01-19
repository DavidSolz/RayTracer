#ifndef CAMERA_H
#define CAMERA_H

#include "Vector3.h"

constexpr float deg2rad = 0.0174532925f;
const Vector3 worldUp = {0.0f , 1.0f ,0.0f};

struct Camera{

    Vector3 front = {0.0f, 0.0f, 1.0f};
    Vector3 up = {0.0f, 1.0f, 0.0f};
    Vector3 right = {1.0f, 0.0f, 0.0f}; 

    Vector3 position = {0.0f, 0.0f, 0.0f};

    float movementSpeed = 100.0f;
    float aspectRatio = 1.0f;
    float nearView = 0.1f;
    float farView = 100.0f;
    float fov = 45.0f;
    float pitch = 0.0f;
    float yaw = 90.0f;

    void Rotate(float offsetX, float offsetY){

        float sensitivity = 0.1f;
        offsetX *= sensitivity;
        offsetY *= sensitivity;

        yaw += offsetX;
        pitch += offsetY;

        float cosPitch = cos(pitch * deg2rad);
        float yaw2rad = yaw * deg2rad;

        Vector3 temp;

        temp.x = cos(yaw2rad) * cosPitch;
        temp.y = -sin(pitch * deg2rad);
        temp.z = sin(yaw2rad) * cosPitch;

        front = (front - temp).Normalize();

        right = Vector3::CrossProduct(worldUp, front).Normalize();
        up  = Vector3::CrossProduct(front, right).Normalize();

    }

    void Move(const Vector3& direction, const float& deltaTime){

        position = position + (Vector3)direction * movementSpeed * deltaTime;

    }

    void LookAt(const Vector3& target){
        front = (target-position).Normalize();
        right = Vector3::CrossProduct(worldUp, front).Normalize();
        up  = Vector3::CrossProduct(front, right).Normalize();
    }

    Vector3 CalculatePixelPosition(const int x, const int y, const int width, const int height){
        float tanHalfFOV = tan(deg2rad * fov * 0.5f);
        float cameraX = (2.0 * x / (float)width - 1.0f) * aspectRatio * tanHalfFOV * nearView;
        float cameraY = (2.0 * y / (float)height - 1.0f) * tanHalfFOV * nearView;
        
        return position + front*nearView + right*cameraX + up*cameraY;

    }
};


#endif