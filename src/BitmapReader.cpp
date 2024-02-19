#include "BitmapReader.h"

static Header header;
static Info infoHeader;


void BitmapReader::ParseData(char * destination, const char *source, unsigned int & offset, const int length){

    const char * localData = source + offset;
    memcpy(destination, localData, length);
    offset += length;

}

void BitmapReader::ParseHeader(const char * source, unsigned int & offset){

    ParseData((char*)&header.signature, source, offset, 2);
    ParseData((char*)&header.fileSize, source, offset, 4);
    ParseData((char*)&header.reserved, source, offset, 4);
    ParseData((char*)&header.dataOffset, source, offset, 4);

}

char BitmapReader::ParseInfo(const char * source, unsigned int & offset){

    ParseData((char*)&infoHeader.size, source, offset, 4);

    if(infoHeader.size!=124){
        fprintf(stderr, "Invalid file format. Only BITMAPV5HEADER are currently handled.\n");
        return -1;
    }

    ParseData((char*)&infoHeader.width, source, offset, 4);
    ParseData((char*)&infoHeader.height, source, offset, 4);
    ParseData((char*)&infoHeader.planes, source, offset, 2);
    ParseData((char*)&infoHeader.bits_per_pixel, source, offset, 2);
    ParseData((char*)&infoHeader.compression, source, offset, 4);
    ParseData((char*)&infoHeader.image_size, source, offset, 4);
    ParseData((char*)&infoHeader.x_pixels_per_m, source, offset, 4);
    ParseData((char*)&infoHeader.y_pixels_per_m, source, offset, 4);
    ParseData((char*)&infoHeader.colors_used, source, offset, 4);
    ParseData((char*)&infoHeader.important_colors, source, offset, 4);
    ParseData((char*)&infoHeader.red_mask, source, offset, 4);
    ParseData((char*)&infoHeader.green_mask, source, offset, 4);
    ParseData((char*)&infoHeader.blue_mask, source, offset, 4);
    ParseData((char*)&infoHeader.alpha_mask, source, offset, 4);
    ParseData((char*)&infoHeader.color_space_type, source, offset, 4);
    ParseData((char*)&infoHeader.red_x, source, offset, 4);
    ParseData((char*)&infoHeader.red_y, source, offset, 4);
    ParseData((char*)&infoHeader.red_z, source, offset, 4);
    ParseData((char*)&infoHeader.green_x, source, offset, 4);
    ParseData((char*)&infoHeader.green_y, source, offset, 4);
    ParseData((char*)&infoHeader.green_z, source, offset, 4);
    ParseData((char*)&infoHeader.blue_x, source, offset, 4);
    ParseData((char*)&infoHeader.blue_y, source, offset, 4);
    ParseData((char*)&infoHeader.blue_z, source, offset, 4);
    ParseData((char*)&infoHeader.gamma_red, source, offset, 4);
    ParseData((char*)&infoHeader.gamma_green, source, offset, 4);
    ParseData((char*)&infoHeader.gamma_blue, source, offset, 4);
    ParseData((char*)&infoHeader.intent, source, offset, 4);
    ParseData((char*)&infoHeader.profile_data, source, offset, 4);
    ParseData((char*)&infoHeader.profile_size, source, offset, 4);
    ParseData((char*)&infoHeader.reserved, source, offset, 4);

    return 0;
}


Image BitmapReader::ReadFile(const char * filename){

    std::fstream input;
    input.open(filename, std::ios::in | std::ios::binary);

    if(!input){
        fprintf(stderr, "File can't be opened.\n");
        return Image();
    }

    input.seekg(0, std::ios::end);
    unsigned int file_size = input.tellg();
    input.seekg(0, std::ios::beg);

    char * raw_data = new char[file_size];

    input.read(raw_data, file_size);
    input.close();

    uint32_t offset = 0;

    ParseHeader(raw_data, offset);

    if( ParseInfo(raw_data, offset) != 0 ){
        return Image();
    }

    uint32_t bytes_per_pixel = infoHeader.bits_per_pixel/8;
    uint32_t pixel_count = infoHeader.width * infoHeader.height;
    uint32_t row_size = (infoHeader.width * bytes_per_pixel + 3) & ~3;

    if(bytes_per_pixel < 3 || bytes_per_pixel > 4){
        fprintf(stderr, "Invalid pixel format.\n");
        delete[] raw_data;
        return Image();
    }

    Color * pixels = new Color[ pixel_count ];

    unsigned char tempColor[4] = {0};

    for (uint32_t y = 0; y < infoHeader.height; ++y) {
        for (uint32_t x = 0; x < infoHeader.width; ++x) {
            ParseData((char*)tempColor, raw_data, offset, bytes_per_pixel);

            uint32_t id = y * infoHeader.width + x;

            pixels[id].A = 1.0f;
            pixels[id].R = tempColor[2] / 255.0f;
            pixels[id].G = tempColor[1] / 255.0f;
            pixels[id].B = tempColor[0] / 255.0f;
        }

        offset += (row_size - infoHeader.width * bytes_per_pixel);
    }

    delete[] raw_data;

    Image image;
    image.width = infoHeader.width;
    image.height = infoHeader.height;
    image.data = pixels;

    return image;
}
