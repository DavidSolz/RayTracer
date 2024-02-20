void kernel AntiAlias(
    write_only image2d_t image, 
    global float4 * scratch 
    ){

    int x = get_global_id(0);
    int y = get_global_id(1);

    int width = get_global_size(0);
    int height = get_global_size(1);

    float4 pixelValue = 0.0f;

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {

            int neighborX = clamp(x + i, 0, width - 1);
            int neighborY = clamp(y + j, 0, height - 1);

            int index = neighborY * width + neighborX;

            pixelValue += scratch[index] ;

        }
    }

    write_imagef(image, (int2)(x,y), pixelValue/9.0f);

}