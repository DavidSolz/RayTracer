#ifndef COLOR_H
#define COLOR_H

typedef unsigned char uchar;

#include <cmath>

struct Color {
    float R;
    float G;
    float B;
    float A;

    Color operator+(const Color& color) const{ 
        return {
            R + color.R,
            G + color.G,
            B + color.B,
            A + color.A
            };
    }

    Color operator+=(const Color& color){
        this->R += color.R;
        this->G += color.G;
        this->B += color.B;
        this->A += color.A;
        return *this;
    }

    Color operator-(const Color& color) const{
        return {
            R - color.R,
            G - color.G,
            B - color.B,
            A - color.A
            };
    }

    Color operator*(float factor) const {
        return {
            R * factor,
            G * factor,
            B * factor,
            A * factor
            };
    }
    

    Color operator*(const Color& other) const {
        return {
            R * other.R,
            G * other.G,
            B * other.B,
            A * other.A
            };
    }

    Color operator*=(const Color& other) {
        
        this->R *= other.R;
        this->G *= other.G;
        this->B *= other.B;
        this->A *= other.A;

        return *this;
    }

    static Color Lerp(const Color & a, const Color & b, const float & t){
        float scale = std::fmax(0.0f, std::fmin(t, 1.0f));
        return a + (b-a)*scale;
    }

    static Color Pack(const unsigned char _R, const unsigned char _G, const unsigned char _B){
        return (Color){_R, _G, _B, 1.0f};
    }

    static void Unpack(const Color & _color, float * array){
        array[0] = _color.R;
        array[1] = _color.G;
        array[2] = _color.B;
        array[3] = 1.0f;
    }

    static float Similarity(const Color& colorA, const Color& colorB){
        float dR = fabs(colorA.R - colorB.R);
        float dG = fabs(colorA.G - colorB.G);
        float dB = fabs(colorA.B - colorB.B);
        float dA = fabs(colorA.A - colorB.A);
        return 1.0f - (dR+dG+dB+dA)/4.0f;
    }

} __attribute__((aligned(16)));

#endif
