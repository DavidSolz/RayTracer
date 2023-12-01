#ifndef CAMERA_H
#define CAMERA_H

#include "Vector3.h"

struct Camera{

    Vector3 front = {0.0f, 0.0f, 1.0f};
    Vector3 up = {0.0f, 1.0f, 0.0f};
    Vector3 right = {1.0f, 0.0f, 0.0f}; 

    Vector3 position = {0.0f, 0.0f, 0.0f};

    float movementSpeed = 1.0f;
    float rotationSpeed = 15.0f;
    float aspectRatio = 1.0f;
    float nearView = 0.1f;
    float farView = 100.0f;
    float fov = 45.0f;

    void Rotate(const float& angle){

        float cosAngle = cos(angle);
        float sinAngle = sin(angle);

        bool isRotating = (angle!=0);

        Vector3 newFront(front.x * cosAngle - front.z * sinAngle, front.y, front.x * sinAngle + front.z * cosAngle);
        Vector3 newRight(right.x * cosAngle - right.z * sinAngle, right.y, -right.x * sinAngle + right.z * cosAngle);

        front = front * (1 - isRotating) + newFront * isRotating;

        right = right * (1 - isRotating) + newRight * isRotating;

    }

    Vector3 CalculatePixelPosition(const int x, const int y, const int width, const int height){
        float tanHalfFOV = tan(3.141569f/180.0f * fov / 2.0);
        float cameraX = (2.0 * x / width - 1.0f) * aspectRatio * tanHalfFOV * nearView;
        float cameraY = (2.0 * y / height - 1.0f) * tanHalfFOV * nearView;
        
        return position + front*nearView + right*cameraX + up*cameraY;

    }
};


#endif