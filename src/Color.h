#ifndef COLOR_H
#define COLOR_H

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

    Color operator-=(const Color& color){
        this->R -= color.R;
        this->G -= color.G;
        this->B -= color.B;
        this->A -= color.A;
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

    /// @brief Mixes colors in equal proportion
    /// @param a 
    /// @param b 
    /// @return mixture of color a and color b
    static Color Mix(const Color & a, const Color & b){
        return a * b;
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

    /// @brief Unpacks coded RGB values to float array
    /// @param _color 
    /// @param _array inout array of colors
    static void Unpack(const unsigned int & _color, float _array[3]){
        _array[0] = (_color>>24 && 255)/ 255.0f;
        _array[1] = (_color>>16 && 255)/ 255.0f;
        _array[2] = (_color>>8 && 255)/ 255.0f;
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

#endif
