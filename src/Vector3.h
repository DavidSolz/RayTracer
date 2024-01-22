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

    Vector3(const float & _x, const float & _y, const float & _z){
        this->x=_x;
        this->y=_y;
        this->z=_z;
    }

    Vector3(const float & _x, const float & _y){
        this->x=_x;
        this->y=_y;
        this->z= 0;
    }

    static float DotProduct(const Vector3 & a, const Vector3 & b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static Vector3 CrossProduct(const Vector3 & a, const Vector3 & b) {
    
        float nX = a.y*b.z - b.y*a.z;
        float nY =  -(a.x*b.z - b.x*a.z);
        float nZ = a.x*b.y - b.x*a.y;

        return Vector3(nX, nY, nZ);
    }

    static Vector3 Lerp(const Vector3 & a, const Vector3 & b, float & t){
        t = fmax(0.0f, fmin(t, 1.0f));
        return a + (b-a)*t;
    }

    static Vector3 Minimal(const Vector3 & a, const Vector3 & b){
        return Vector3(fmin(a.x,b.x), fmin(a.y,b.y), fmin(a.z,b.z));
    }

    static Vector3 Maximal(const Vector3 & a, const Vector3 & b){
        return Vector3(fmax(a.x,b.x), fmax(a.y,b.y), fmax(a.z,b.z));
    }

    Vector3 operator-(const Vector3 & b) const {
        return Vector3(x - b.x, y - b.y, z - b.z);
    }

    Vector3 operator+(const Vector3 & b) const {
        return Vector3(x + b.x, y + b.y, z + b.z);
    }

    Vector3 operator*(const float & scalar) const{
        return Vector3(x*scalar, y*scalar, z*scalar);
    }

    Vector3 operator*(const Vector3 & other) const {
        return Vector3(x*other.x, y*other.y, z*other.z);
    }

    bool operator<(const Vector3 & other) const{
        return x<other.x && y<other.y && z<other.z;
    }

    float Magnitude() const {
        return sqrt(x*x+y*y+z*z);
    }

    Vector3 Directions() const{
        return Vector3(2*(x>0)-1, 2*(y>0)-1, 2*(z>0)-1);
    }

    Vector3 Absolute() const{
        return Vector3(fabs(x), fabs(y), fabs(z));
    }

    Vector3 Normalize() const{
        float magnitude = Magnitude();
        return Vector3{x/magnitude, y/magnitude, z/magnitude};
    }

} __attribute__((aligned(16)));


#endif