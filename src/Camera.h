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

    float movementSpeed = 1000.0f;
    float rotationSpeed = 100.0f;
    float aspectRatio = 1.0f;
    float nearView = 0.1f;
    float farView = 100.0f;
    float fov = 45.0f;
    float pitch = 0.0f;
    float yaw = 90.0f;

    void Rotate(double offsetX, double offsetY){

        yaw += offsetX * rotationSpeed;
        pitch += offsetY * rotationSpeed;

        pitch = fmax(-89.0f, fmin(pitch, 89.0f));

        float pitch2rad = pitch * deg2rad;
        float yaw2rad = yaw * deg2rad;

        front.x = cos(yaw2rad) * cos(pitch2rad);
        front.y = sin(pitch2rad);
        front.z = sin(yaw2rad) * cos(pitch2rad);

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
