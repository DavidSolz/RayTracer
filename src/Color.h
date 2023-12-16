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

    Color operator*(float factor) const {
        factor = std::fmax(0.0f, std::fmax(factor, 1.0f));
        return {
            R * factor,
            G * factor,
            B * factor,
            A * factor
            };
    }

    static Color LerpColor(struct Color colorA, struct Color colorB, float t){
        t = fmax(0.0f, fmin(t, 1.0f));
        return (struct Color){
            colorA.R + t*(colorB.R - colorA.R),
            colorA.G + t*(colorB.G - colorA.G),
            colorA.B + t*(colorB.B - colorA.B),
            fmax(colorA.A, colorB.A)
        };
    }

    static Color MixColors(struct Color colorA, struct Color colorB) {
        return (struct Color){
            colorA.R * colorB.R,
            colorA.G * colorB.G,
            colorA.B * colorB.B,
            colorA.A * colorB.A
        };
    }

    static Color ToneColor(struct Color color, float factor) {
        factor = fmax(0.0f, fmin(factor, 1.0f));
        return (struct Color){
            color.R * factor,
            color.G * factor,
            color.B * factor,
            color.A * factor
        };
    }

} __attribute__((aligned(16)));

#endif
