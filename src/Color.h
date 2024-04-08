#ifndef COLOR_H
#define COLOR_H

#include <cmath>
#include <immintrin.h>

constexpr float ONE_OVER_UCHAR_MAX = 1.0f/255.0f;

struct Color {
    float R;
    float G;
    float B;
    float A;

    Color operator+(const Color& color) const{

        __m128 first = _mm_load_ps((float*)this);
        __m128 second = _mm_load_ps((float*)&color);
        __m128 sum = _mm_add_ps(first, second);

        Color result;
        _mm_store_ps((float*)&result, sum);

        return result;
    }

    Color operator+=(const Color& color){
        this->R += color.R;
        this->G += color.G;
        this->B += color.B;
        this->A += color.A;
        return *this;
    }

    Color operator-=(const Color& color){
        this->R -= color.R;
        this->G -= color.G;
        this->B -= color.B;
        this->A -= color.A;
        return *this;
    }

    Color operator-(const Color& color) const{

        __m128 first = _mm_load_ps((float*)this);
        __m128 second = _mm_load_ps((float*)&color);
        __m128 sub = _mm_sub_ps(first, second);

        Color result;
        _mm_store_ps((float*)&result, sub);

        return result;
    }

    Color operator*(float factor) const {

        __m128 constant = _mm_set1_ps(factor);
        __m128 current = _mm_loadu_ps((float*)this);
        __m128 mul = _mm_mul_ps(current, constant);

        Color result;
        _mm_store_ps((float*)&result, mul);

        return result;
    }


    Color operator*(const Color& color) const {

        __m128 current = _mm_loadu_ps((float*)this);
        __m128 other = _mm_loadu_ps((float*)&color);
        __m128 mul = _mm_mul_ps(current, other);

        Color result;
        _mm_store_ps((float*)&result, mul);

        return result;
    }

    Color operator*=(const Color& other) {

        this->R *= other.R;
        this->G *= other.G;
        this->B *= other.B;
        this->A *= other.A;

        return *this;
    }

    /// @brief Lineary interpolates between two colors
    /// @param a first color
    /// @param b second color
    /// @param t interpolation scale
    /// @return lineary interpolated color between a and b
    static Color Lerp(const Color & a, const Color & b, const float & t){
        float scale = std::fmax(0.0f, std::fmin(t, 1.0f));
        return a + (b-a)*scale;
    }

    /// @brief Packs RGB values to single int where A is always 255
    /// @param _R red amount
    /// @param _G green amount
    /// @param _B blue amount
    /// @return color packind in single int
    static unsigned int Pack(const unsigned char _R, const unsigned char _G, const unsigned char _B){
        return _R<<24 | _G <<16 | _B << 8 | 255 ;
    }

    static Color Clamp(const Color & color){

        __m128 current = _mm_load_ps((float*)&color);
        __m128 zeros = _mm_setzero_ps();
        __m128 ones = _mm_set1_ps(1.0f);

        __m128 clamped = _mm_min_ps(current, ones);
        clamped = _mm_max_ps(clamped, zeros);

        Color result;
        _mm_store_ps((float*)&result, clamped);

        return result;
    }

    /// @brief Unpacks coded RGB values to float array
    /// @param _color
    /// @param _array inout array of colors
    static void Unpack(const unsigned int & _color, float _array[3]){
        _array[0] = ( (_color>>24) & 255) * ONE_OVER_UCHAR_MAX;
        _array[1] = ( (_color>>16) & 255) * ONE_OVER_UCHAR_MAX;
        _array[2] = ( (_color>>8) & 255) * ONE_OVER_UCHAR_MAX;
    }

    /// @brief Determines similarity between two colors
    /// @param _colorA
    /// @param _colorB
    /// @return value in range <0, 1> representing similarity
    static float Similarity(const Color& _colorA, const Color& _colorB){
        float dR = fabs(_colorA.R - _colorB.R);
        float dG = fabs(_colorA.G - _colorB.G);
        float dB = fabs(_colorA.B - _colorB.B);
        float dA = fabs(_colorA.A - _colorB.A);
        return 1.0f - (dR+dG+dB+dA)/4.0f;
    }

} __attribute__((aligned(16)));

const static Color WHITE = {1.0f, 1.0f, 1.0f, 1.0f};

#endif
