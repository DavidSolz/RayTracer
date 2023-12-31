#ifndef VECTOR3_H
#define VECTOR3_H

#include <cmath>

struct Vector3 {
    float x;
    float y;
    float z;

    Vector3(){
        this->x=0;
        this->y=0;
        this->z=0;
    }

    Vector3(const float& _x, const float& _y, const float& _z){
        this->x=_x;
        this->y=_y;
        this->z=_z;
    }

    Vector3(const float& _x, const float& _y){
        this->x=_x;
        this->y=_y;
        this->z= 0;
    }

    static float DotProduct(const Vector3& a, const Vector3& b){
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static Vector3 CrossProduct(const Vector3& a, const Vector3& b){
    
        float nX = a.y*b.z - b.y*a.z;
        float nY =  -(a.x*b.z - b.x*a.z);
        float nZ = a.x*b.y - b.x*a.y;

        return Vector3(nX, nY, nZ);
    }

    static Vector3 Lerp(const Vector3& a, const Vector3& b, float & t){
        t = fmax(0.0f, fmin(t, 1.0f));
        return a + (b-a)*t;
    }

    Vector3 operator-(const Vector3& b) const {
        return Vector3(x - b.x, y - b.y, z - b.z);
    }

    Vector3 operator+(const Vector3& b) const {
        return Vector3(x + b.x, y + b.y, z + b.z);
    }

    Vector3 operator*(const float& scalar){
        return Vector3(x*scalar, y*scalar, z*scalar);
    }

    float Magnitude(){
        return sqrt(x*x+y*y+z*z);
    }

    Vector3 Normalize(){
        float magnitude = Magnitude();
        return Vector3{x/magnitude, y/magnitude, z/magnitude};
    }

} __attribute__((aligned(16)));


#endif