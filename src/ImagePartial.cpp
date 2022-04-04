#include "ImagePartial.h"

ImagePartial::ImagePartial() {
    width = -1;
    height = -1;
    bit_depth = -1;
    color_type = -1;
    compression_method = -1;
    filler_method = -1;
    interlace_method = -1;
    next_pixel_pos = {0, 0};
    std::vector<std::vector<uint32_t>> image_data;
}

void ImagePartial::set_next_pixel(uint32_t pixel) {
    image_data[next_pixel_pos.height][next_pixel_pos.width] = pixel;
}

void ImagePartial::advance() {
    // no interlace, width is fastest moving index
    if (interlace_method == 0) {
        next_pixel_pos.width++;
        if (next_pixel_pos.width == width) {
            next_pixel_pos.width = 0;
            next_pixel_pos.height++;
        }
    } else { // Adam7 interlace
    }

}

std::vector<std::vector<uint32_t>> ImagePartial::get_image() {
    return image_data; 
}
