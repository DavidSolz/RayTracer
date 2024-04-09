#ifndef SHADING_H
#define SHADING_H

#include "Vector3.h"
#include "Material.h"
#include "Object.h"
#include "Texture.h"
#include "Color.h"

#define ALPHA_MIN 0.001f
#define ONE_OVER_PI 1.0f/3.1415926535f

namespace Shading {

float SchlickFresnel(const float & value){
    float temp = 1.0f - value;
    return temp * temp * temp * temp * temp;
}

Color Tint(const Color & albedo){
    float luminance = albedo.R * 0.3f + albedo.G * 0.6f + albedo.B;
    float condition = luminance > 0.0f;
    return Color::Lerp(WHITE, albedo * (1.0f/luminance) , condition);
}

Color Sheen(const float & cosLightHalf, const struct Material & material){
    Color tint = Tint(material.albedo);
    Color sheen = Color::Lerp(WHITE, tint, material.tintRoughness);
    return sheen * SchlickFresnel(cosLightHalf) * material.sheen;
}

float DiffuseBRDF(const float & cosView, const float & cosLight, const struct Material & material){

    float FL = SchlickFresnel(cosLight);
    float FV = SchlickFresnel(cosView);

    float R = 0.5f + 2.0f * cosLight * cosLight * material.roughness * material.roughness;
    float retro = R * ( FL + FV + FL * FV * (R - 1.0f) );

    return ONE_OVER_PI * ( (1.0f - 0.5f * FL) * (1.0f - 0.5f * FV) + retro );
}

float GgxAnisotropic(const Vector3 & halfVector, const float & ax, const float & ay){

    float dotHX2 = halfVector.x * halfVector.x;
    float dotHY2 = halfVector.z * halfVector.z;
    float cos2Theta = cos(halfVector.y) * cos(halfVector.y);
    float ax2 = ax * ax;
    float ay2 = ay * ay;

    float temp = (dotHX2 / ax2 + dotHY2 / ay2 + cos2Theta);

    return ONE_OVER_PI * 1.0f / (ax * ay * temp * temp);
}

float SeparableSmithGGXG1BSDF(const Vector3 & vector, const Vector3 & halfVector, const float & ax, const float & ay){

    float cos2Theta = halfVector.y * halfVector.y;
    float sin2Theta = 1.0f - cos2Theta;

    float tanTheta = sqrt( sin2Theta/cos2Theta );

    float cos2Phi = vector.x * vector.x;
    float sin2Phi = 1.0f - cos2Phi;

    float a = sqrt( cos2Phi * ax * ax + sin2Phi * ay * ay);
    float a2Tan2Theta = a * a * tanTheta * tanTheta;

    float lambda = 0.5f * (-1.0f + sqrt( 1.0f + a2Tan2Theta ));
    return 1.0f / (1.0f + lambda);

}

float SpecularBSDF(const Vector3 & normal, const Vector3 & lightVector, const Vector3 & viewVector, const Vector3 & halfVector, const struct Material & material){

    float aspect = sqrt(1.0f - 0.9f * material.anisotropy);

    float roughnessSqr = material.roughness * material.roughness;

    float ax = fmax(ALPHA_MIN, roughnessSqr / aspect);
    float ay = fmax(ALPHA_MIN, roughnessSqr * aspect);

    float cosLight = Vector3::DotProduct(normal, lightVector);
    float cosView = Vector3::DotProduct(normal, viewVector);

    float D = GgxAnisotropic(halfVector, ax, ay);
    float Gl = SeparableSmithGGXG1BSDF(lightVector, halfVector, ax, ay);
    float Gv = SeparableSmithGGXG1BSDF(viewVector, halfVector, ax, ay);

    return D * Gl * Gv / (4.0f * cosLight * cosView);
}

Color SpecularTransmissionBSDF(const Vector3 & lightVector, const Vector3 & viewVector, const Vector3 & halfVector, const struct Material & material){
    float aspect = sqrt(1.0f - 0.9f * material.anisotropy);

    float roughnessSqr = material.roughness * material.roughness;

    float ax = fmax(ALPHA_MIN, roughnessSqr / aspect);
    float ay = fmax(ALPHA_MIN, roughnessSqr * aspect);

    float cosViewHalf = Vector3::DotProduct(viewVector, halfVector);

    cosViewHalf *= halfVector.y;

    float eta = 1.0f / material.indexOfRefraction;

    float D = GgxAnisotropic(halfVector, ax, ay);
    float Gl = SeparableSmithGGXG1BSDF(lightVector, halfVector, ax, ay);
    float Gv = SeparableSmithGGXG1BSDF(viewVector, halfVector, ax, ay);
    float F = eta + (1.0f - eta) * SchlickFresnel(cosViewHalf);

    float result = D * F * Gl * Gv;

    return {result, result, result, result};
}

float GTR(const float & cosLightHalf, const float & alpha){

    if( alpha >= 1.0f)
        return ONE_OVER_PI;

    float alphaSqr = alpha * alpha;
    float decAlphaSqr = alphaSqr - 1.0f;

    return ONE_OVER_PI * decAlphaSqr/( log2(alphaSqr)*(1.0f + decAlphaSqr * cosLightHalf * cosLightHalf) );
}

float SeparableSmithGGXG1(const float & cosine, const float & alpha){
    float a2 = alpha * alpha;
    return 2.0f / (1.0f + sqrt(a2 + (1 - a2) * cosine * cosine));
}

Color ClearcoatBRDF(const Vector3 & viewVector, const Vector3 & lightVector, const Vector3 & halfVector, const struct Material & material) {

    float cosHalf = fabs(halfVector.y);
    float cosView = fabs(viewVector.y);
    float cosLight = fabs(lightVector.y);
    float cosLightHalf = Vector3::DotProduct(lightVector, halfVector);

    float scale = 0.1f + (0.001f - 0.1f) * material.clearcoatRoughness;

    float D = GTR(cosHalf, scale);
    float Gl = SeparableSmithGGXG1(cosLight, 0.25f);
    float Gv = SeparableSmithGGXG1(cosView, 0.25f);
    float F = 0.04f + 0.96f * SchlickFresnel(cosLightHalf);

    float result = 0.25f * D * Gl * Gv * F;

    return {result, result, result, result};
}

Color Unpack(const unsigned int & color){
    unsigned char * byte = (unsigned char *)&color;
    return (Color){(float)byte[0], (float)byte[1], (float)byte[2], (float)byte[3]} * ONE_OVER_UCHAR_MAX;
}

Color BilinearFilter(
    const unsigned int * texture,
    const float & u,
    const float & v,
    const int & width,
    const int & height,
    const int & offset
) {

    float texCoord[2] = {u * (width - 1), v * (height - 1)};
    float texel[2] = {floor(texCoord[0]), floor(texCoord[1])};
    float frac[2] = {texCoord[0] - texel[0], texCoord[1] - texel[1]};

    Color texelColor00 = Unpack(texture[offset + (int)texel[1] * width + (int)texel[0]]);
    Color texelColor10 = Unpack(texture[offset + (int)texel[1] * width + (int)texel[0] + 1]);
    Color texelColor01 = Unpack(texture[offset + ((int)texel[1] + 1) * width + (int)texel[0]]);
    Color texelColor11 = Unpack(texture[offset + ((int)texel[1] + 1) * width + (int)texel[0] + 1]);

    Color color =
        texelColor00 * (1.0f - frac[0]) * (1.0f - frac[1]) +
        texelColor10 * frac[0] * (1.0f - frac[1]) +
        texelColor01 * (1.0f - frac[0]) * frac[1] +
        texelColor11 * frac[0] * frac[1];

    return color;
}

Color ColorSample(
    const unsigned int * texture,
    const float & u,
    const float & v,
    const int & width,
    const int & height,
    const float & offset
    ){
    return BilinearFilter(texture, u, v, width, height, offset);
}

Color GetTexturePixel(
    const unsigned int * texture,
    const struct Object & object,
    const struct Texture & info,
    const Vector3 & localPoint,
    const Vector3 & normal
    ){

    Vector3 texCoord;

    if ( object.type == SPHERE){
        float theta = atan2(normal.z, normal.x) + 3.1415926535f;
        float phi = acos(normal.y);

        texCoord.x = theta;
        texCoord.y = phi;
        texCoord = texCoord * ONE_OVER_PI;

    }else if( object.type == TRIANGLE ){

        Vector3 A = object.vertices[0];
        Vector3 B = object.vertices[1];
        Vector3 C = object.vertices[2];

        float area = ((B.x - A.x) * (C.y - A.y) - (B.y - A.y) * (C.x - A.x)) * 0.5f;
        float areaPBC = ((B.x - localPoint.x) * (C.y - localPoint.y) - (B.y - localPoint.y) * (C.x - localPoint.x)) * 0.5f;
        float areaPCA = ((C.x - localPoint.x) * (A.y - localPoint.y) - (C.y - localPoint.y) * (A.x - localPoint.x)) * 0.5f;
        float areaPAB = ((A.x - localPoint.x) * (B.y - localPoint.y) - (A.y - localPoint.y) * (B.x - localPoint.x)) * 0.5f;

        texCoord.x = areaPBC / area;
        texCoord.y = areaPCA / area;
        texCoord.z = 1.0f - texCoord.x - texCoord.y;

        texCoord = Vector3::Clamp(texCoord);
    }

    return ColorSample(texture, texCoord.x, texCoord.y, info.width, info.height, info.offset);
}

}

#endif
