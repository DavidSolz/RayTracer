#ifndef COLOR_H
#define COLOR_H

typedef unsigned char uchar;

#include <cmath>

struct Color {
    float R;
    float G;
    float B;
    float A;

    Color operator+(const Color& color){
        return {
            R + color.R,
            G + color.G,
            B + color.B,
            A + color.A
            };
    }

    Color operator-(const Color& color){
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

    static float Similarity(const Color& colorA, const Color& colorB){
        float dR = colorA.R - colorB.R;
        float dG = colorA.G - colorB.G;
        float dB = colorA.B - colorB.B;
        float dA = colorA.A - colorB.A;
        return sqrt( dR*dR + dG*dG + dB*dB + dA*dA );
    }

} __attribute__((aligned(16)));

#endif
