#include "resources/kernels/KernelStructs.h"

#define KERNEL_SIZE 3
#define KERNEL_SUM 4.0f
#define INV_SUM 1.0f / KERNEL_SUM

void kernel Filter(write_only image2d_t image, global struct Resources * resources ){

    local struct Resources localResources;
    localResources = *resources;

    int2 coord = (int2)(get_global_id(0), get_global_id(1));

    int width = localResources.width;
    int height = localResources.height;

    float4 color = 0.0f;

    int filter [KERNEL_SIZE * KERNEL_SIZE] = {1, -1, 1, -1, 4, -1, 1, -1, 1};

    int half_kernel_size = KERNEL_SIZE>>1;

    for (int i = -half_kernel_size; i <= half_kernel_size; ++i){
        for (int j = -half_kernel_size; j <= half_kernel_size; ++j){
            
            int2 currentPixel = coord + (int2)(i, j);

            currentPixel.x = clamp(currentPixel.x, 0, width - 1);
            currentPixel.y = clamp(currentPixel.y, 0, height - 1);

            int index = currentPixel.y * width + currentPixel.x;

            float4 sample = localResources.colors[ index ];

            int kernelX = i + half_kernel_size;
            int kernelY = j + half_kernel_size;

            color += sample * filter[ kernelY * KERNEL_SIZE + kernelX ];
        }
    }

    write_imagef(image, coord, color * INV_SUM);
}
