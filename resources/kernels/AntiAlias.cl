void kernel AntiAlias(
    write_only image2d_t image, 
    global float4 * scratch 
    ){

    int2 coord = (int2)(get_global_id(0), get_global_id(1));

    int width = get_global_size(0);
    int height = get_global_size(1);

    float4 colorVector = 0.0f;

    for (int i = -1; i <= 1; ++i){
        for (int j = -1; j <= 1; ++j){
            
            int2 currentPixel = coord + (int2)(i, j);

            currentPixel.x = clamp(currentPixel.x, 0, width - 1);
            currentPixel.y = clamp(currentPixel.y, 0, height - 1);

            int index = currentPixel.y * width + currentPixel.x;

            float4 sample = scratch[ index ];

            colorVector += sample;

        }
    }

    write_imagef(image, coord, colorVector / 9.0f);
}