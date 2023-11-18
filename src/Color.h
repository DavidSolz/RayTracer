#ifndef COLOR_H
#define COLOR_H

typedef unsigned char uchar;

struct Color {
    uchar R;
    uchar G;
    uchar B;
    uchar A;

    Color operator+(const Color& color){
        return {
            std::min(R + color.R, 255),
            std::min(G + color.G, 255),
            std::min(B + color.B, 255),
            std::min(A + color.A, 255)
        };
    }

    Color operator*(float factor) const {
        return {
            std::fmin(R * factor, 255.0f),
            std::fmin(G * factor, 255.0f),
            std::fmin(B * factor, 255.0f),
            std::fmin(A * factor, 255.0f)
        };
    }

    static Color Max(const Color &a, const Color &b){
        return {
            std::max(a.R, b.R),
            std::max(a.G, b.G),
            std::max(a.B, b.B),
            std::max(a.A, b.A)
        };
    }

    static Color Mix(const Color& a, const Color& b, float factor) {
        factor = std::max(0.0f, std::min(factor, 1.0f));

        return {
            (uchar)((1.0f - factor) * a.R + factor * b.R),
            (uchar)((1.0f - factor) * a.G + factor * b.G),
            (uchar)((1.0f - factor) * a.B + factor * b.B),
            (uchar)((1.0f - factor) * a.A + factor * b.A)
        };
    }

};

#endif