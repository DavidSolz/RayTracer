#ifndef VECTOR3_H
#define VECTOR3_H

#include <cmath>
#include <immintrin.h>

struct Vector3 {
    float x;
    float y;
    float z;
    float w;

    Vector3(){
        this->x=0.0f;
        this->y=0.0f;
        this->z=0.0f;
        this->w=0.0f;
    }

    Vector3(const float & _x, const float & _y, const float & _z){
        this->x = _x;
        this->y = _y;
        this->z = _z;
        this->w = 0.0f;
    }

    Vector3(const float & _x, const float & _y){
        this->x = _x;
        this->y = _y;
        this->z = 0.0f;
        this->w = 0.0f;
    }

    static float DotProduct(const Vector3 & a, const Vector3 & b) {
        return a.x*b.x + a.y*b.y + a.z*b.z;
    }

    static Vector3 Root(const Vector3 & a) {
        return Vector3(sqrtf(a.x), sqrtf(a.y), sqrtf(a.z));
    }

    static Vector3 CrossProduct(const Vector3 & a, const Vector3 & b) {
        float nX = a.y*b.z - b.y*a.z;
        float nY = -(a.x*b.z - b.x*a.z);
        float nZ = a.x*b.y - b.x*a.y;
        return Vector3(nX, nY, nZ);
    }

    static Vector3 Lerp(const Vector3 & a, const Vector3 & b, float & t){
        t = fmax(0.0f, fmin(t, 1.0f));
        return a + (b-a)*t;
    }

    static Vector3 Minimal(const Vector3 & a, const Vector3 & b){

        __m128 first = _mm_load_ps((float*)&a);
        __m128 second = _mm_load_ps((float*)&b);
        __m128 min = _mm_min_ps(first, second);

        Vector3 result;
        _mm_store_ps((float*)&result, min);

        return result;
    }

    static Vector3 Maximal(const Vector3 & a, const Vector3 & b){

        __m128 first = _mm_load_ps((float*)&a);
        __m128 second = _mm_load_ps((float*)&b);
        __m128 max = _mm_max_ps(first, second);

        Vector3 result;
        _mm_store_ps((float*)&result, max);

        return result;
    }

    Vector3 operator-(const Vector3 & b) const {
        __m128 first = _mm_load_ps((float*)this);
        __m128 second = _mm_load_ps((float*)&b);
        __m128 sub = _mm_sub_ps(first, second);

        Vector3 result;
        _mm_store_ps((float*)&result, sub);

        return result;
    }

    Vector3 operator-(const float & scalar) const {

        __m128 constant = _mm_load_ps((float*)this);
        __m128 current = _mm_set1_ps(scalar);
        __m128 sub = _mm_sub_ps(current, constant);

        Vector3 result;
        _mm_store_ps((float*)&result, sub);

        return result;
    }

    Vector3 operator+(const Vector3 & b) const {

        __m128 first = _mm_load_ps((float*)this);
        __m128 second = _mm_load_ps((float*)&b);
        __m128 sum = _mm_add_ps(first, second);

        Vector3 result;
        _mm_store_ps((float*)&result, sum);

        return result;
    }

    Vector3 operator+(const float & scalar) const {

        __m128 constant = _mm_load_ps((float*)this);
        __m128 current = _mm_set1_ps(scalar);
        __m128 sum = _mm_add_ps(current, constant);

        Vector3 result;
        _mm_store_ps((float*)&result, sum);

        return result;
    }

    Vector3 operator*(const float & scalar) const{

        __m128 current = _mm_load_ps((float*)this);
        __m128 constant = _mm_set1_ps(scalar);
        __m128 mul = _mm_mul_ps(current, constant);

        Vector3 result;
        _mm_store_ps((float*)&result, mul);

        return result;
    }

    Vector3 operator/(const float & scalar) const{

        __m128 current = _mm_load_ps((float*)this);
        __m128 constant = _mm_set1_ps(1.0f/scalar);
        __m128 mul = _mm_mul_ps(current, constant);

        Vector3 result;
        _mm_store_ps((float*)&result, mul);

        return result;
    }

    Vector3 operator*(const Vector3 & other) const {

        __m128 first = _mm_load_ps((float*)this);
        __m128 second = _mm_load_ps((float*)&other);
        __m128 mul = _mm_mul_ps(first, second);

        Vector3 result;
        _mm_store_ps((float*)&result, mul);

        return result;
    }

    bool operator<(const Vector3 & other) const{
        return x<other.x && y<other.y && z<other.z;
    }

    float operator[](const int & id) const{
        return x * (id == 0) + y * (id == 1) + z * (id == 2);
    }

    float Magnitude() const {
        return sqrtf( Vector3::DotProduct(*this, *this) );
    }

    Vector3 Directions() const{
        return Vector3(2.0f * ( x > 0.0f) - 1.0f, 2.0f * ( y > 0.0f ) - 1.0f, 2.0f * ( z > 0.0f ) - 1.0f);
    }

    Vector3 Absolute() const{
        return Vector3(fabs(x), fabs(y), fabs(z));
    }

    Vector3 Normalize() const{
        float magnitude = Magnitude();
        return *this/magnitude;
    }

} __attribute__((aligned(16)));


#endif
