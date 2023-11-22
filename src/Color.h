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

    Color BalanceColor(struct Color colorA, struct Color colorB, float t){
        t = fmax(0.0f, fmin(t, 1.0f));
        return (struct Color){
            (1-t)*colorA.R + t*colorB.R,
            (1-t)*colorA.G + t*colorB.G,
            (1-t)*colorA.B + t*colorB.B,
            (1-t)*colorA.A + t*colorB.A
        };
    }

    Color LerpColor(struct Color colorA, struct Color colorB, float t){
        t = fmax(0.0f, fmin(t, 1.0f));
        return (struct Color){
            colorA.R + (colorB.R - colorA.R) * t,
            colorA.G + (colorB.R - colorA.G) * t,
            colorA.B + (colorB.R - colorA.B) * t,
            colorA.A + (colorB.R - colorA.A) * t
        };
    }

    Color MixColors(struct Color colorA, struct Color colorB) {
        return (struct Color){
            colorA.R * colorB.R,
            colorA.G * colorB.G,
            colorA.B * colorB.B,
            colorA.A * colorB.A
        };
    }

    struct Color MaxColor(struct Color colorA, struct Color colorB){

        bool greater =  colorA.R > colorB.R &&
                        colorA.G > colorB.G &&
                        colorA.B > colorB.B ;

        return  colorA * greater + colorB * (1-greater);
    }

};

#endif
