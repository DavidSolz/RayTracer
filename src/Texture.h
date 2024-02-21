#ifndef TEXTURE_H
#define TEXTURE_H

struct Texture{
    int width;
    int height;
    int offset;
    int normalWidth;
    int normalHeight;
    int normalsOffset;
    unsigned int checksum;
} __attribute((aligned(32)));

#endif