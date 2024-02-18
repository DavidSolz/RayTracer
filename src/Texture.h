#ifndef TEXTURE_H
#define TEXTURE_H

struct Texture{
    int width;
    int height;
    int offset;
} __attribute((aligned(16)));

#endif