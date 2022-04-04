#include <iostream>
#include "IHDR.h"
#include "consume.h"
#include "ImagePartial.h"

#define DEBUG 1

void IHDR::print() {
    std::cout << "IHDR:" << std::endl;
    std::cout << "\twidth: " << width << std::endl;
    std::cout << "\theight: " << height << std::endl;
    std::cout << "\tbit depth: " << (int)bit_depth << std::endl;
    std::cout << "\tcolor_type: " << (int)color_type << std::endl;
    std::cout << "\tcompression_method: " << (int)compression_method << std::endl;
    std::cout << "\tfiller_method: " << (int)filler_method << std::endl;
    std::cout << "\tinterlace_method: " << (int)interlace_method << std::endl;
}

IHDR::IHDR(uint32_t _size, std::string _name, std::vector<uint8_t>::iterator &buffer_it) {
    size = _size;
    name = _name;
    width = consume_uint32_t(buffer_it),
    height = consume_uint32_t(buffer_it),
    bit_depth = consume_uint8_t(buffer_it),
    color_type = consume_uint8_t(buffer_it),
    compression_method = consume_uint8_t(buffer_it),
    filler_method = consume_uint8_t(buffer_it),
    interlace_method = consume_uint8_t(buffer_it),
    advance(buffer_it, 4);
}

void IHDR::modify(ImagePartial& image_partial) {
   image_partial.width = width;
   image_partial.height = height;
   image_partial.bit_depth = bit_depth;
   image_partial.color_type = color_type;
   image_partial.compression_method = compression_method;
   image_partial.filler_method = filler_method;
   image_partial.interlace_method = interlace_method;
   image_partial.next_pixel_pos = {0, 0};

   // allocate space for the actual image data
   image_partial.image_data.resize(height, std::vector<uint32_t>(width));
}
