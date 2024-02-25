void kernel AntiAlias(
    write_only image2d_t image, 
    global float4 * scratch 
    ){

    int2 coord = (int2)(get_global_id(0), get_global_id(1));
    int width = get_global_size(0);
    int height = get_global_size(1);
    int size = width * height;
    int idx = coord.y * width + coord.x;

    int top = clamp(coord.y - 1, 0, height - 1) * width + coord.x;
    int bot = clamp(coord.y + 1, 0, height - 1) * width + coord.x;
    int left = coord.y * width + clamp(coord.x - 1, 0, width - 1);
    int right = coord.y * width + clamp(coord.x + 1, 0, width - 1);

    float4 pixelValue = scratch[idx] + scratch[top] + scratch[bot] + scratch[left] + scratch[right];
    write_imagef(image, coord, pixelValue / 5.0f);

}