#include <iostream>
#include <iomanip>
#include <cstdint>
#include <vector>
#include "IDAT.h"
#include "zlibutils.h"
#include "dumputils.h"
#include "bitstream.h"

#define DEBUG 1


IDAT::IDAT(uint32_t _size, std::string _name, std::vector<uint8_t>::iterator &buffer_it) {
    size = _size;
    name = _name;
    
    uncompressed_bytes = uncompress_zlib_stream(size, buffer_it);
    
    dump_bytes("uncompressed_bytes.raw", uncompressed_bytes.begin(), uncompressed_bytes.end());

    advance(buffer_it, 4); // CRC

#if DEBUG
    std::cout << "Uncompressed bytes" << std::endl;
    for (auto it : uncompressed_bytes) {
        std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)it << std::dec << " ";
    }
    std::cout << std::endl;
#endif

}

void IDAT::print() {
};

void IDAT::modify(ImagePartial& image_partial) {
    switch (image_partial.color_type) {
        case 0:
            modify_0(image_partial);
            break;
        case 2:
            modify_2(image_partial);
            break;
        case 3:
            modify_3(image_partial);
            break;
        case 4:
            modify_4(image_partial);
            break;
        case 6:
            modify_6(image_partial);
            break;
    }
}

void IDAT::modify_0(ImagePartial& image_partial) {
    int pos = 0;
    int n_scanline_bits = 8 + image_partial.width * image_partial.bit_depth;
    int filter_type = 0;
    uint16_t gray_scale_value;
    auto buffer_it = uncompressed_bytes.begin();
    // we need to read bits into pixels
    // calling set pixel
    // until end of stream
    while (pos < uncompressed_bytes.size() * 8) {
        if (!(pos % n_scanline_bits)) {
            // read filter type
            filter_type = slice_bits(pos, pos+8, buffer_it);
#if DEBUG
            std::cout << "Filter Type: " << filter_type << std::endl;
#endif
            pos+=8;
        } else {
            gray_scale_value = slice_bits(pos, pos+image_partial.bit_depth, buffer_it);
            pos += image_partial.bit_depth;

            // TODO: handle 16 bit samples
            uint32_t pixel = 0;
            pixel |= (0xFF << 24); // full opacity
            pixel |= (gray_scale_value << 0);
            pixel |= (gray_scale_value << 8);
            pixel |= (gray_scale_value << 16);
            image_partial.set_next_pixel(pixel);
        }
    }
}
void IDAT::modify_2(ImagePartial& image_partial) {
    int pos = 0;
    int n_scanline_bits = 8 + (image_partial.width * (image_partial.bit_depth * 3));
    int filter_type = 0;
    auto buffer_it = uncompressed_bytes.begin();
    // we need to read bits into pixels
    // calling set pixel
    // until end of stream
    while (pos < uncompressed_bytes.size() * 8) {
        if (!(pos % n_scanline_bits)) {
            // read filter type
            filter_type = slice_bits(pos, pos+8, buffer_it);
#if DEBUG
            std::cout << "Filter Type: " << filter_type << std::endl;
#endif
            pos+=8;
        } else {
            // TODO: handle 16 bit samples
            uint32_t pixel = slice_bits(pos, pos+image_partial.bit_depth*3, buffer_it);
            pos += image_partial.bit_depth * 3;
            pixel = pixel << 8;
            pixel |= (255); // full opacity
            image_partial.set_next_pixel(pixel);
        }
    }
}

void IDAT::modify_3(ImagePartial& image_partial) {
    int pos = 0;
    int n_scanline_bits = 8 + image_partial.width * image_partial.bit_depth;
    int filter_type = 0;
    uint8_t palette_index;
    auto buffer_it = uncompressed_bytes.begin();
    // we need to read bits into pixels
    // calling set pixel
    // until end of stream
    while (pos < uncompressed_bytes.size() * 8) {
        if (!(pos % n_scanline_bits)) {
            // read filter type
            filter_type = slice_bits(pos, pos+8, buffer_it);
#if DEBUG
            std::cout << "Filter Type: " << filter_type << std::endl;
#endif
            pos+=8;
        } else {
            palette_index = slice_bits(pos, pos+image_partial.bit_depth, buffer_it);
            pos += image_partial.bit_depth;

            uint32_t pixel = image_partial.palette[palette_index];
            std::cout << "Called set next pixel with value " << pixel << std::endl;
            image_partial.set_next_pixel(pixel);
        }
    }

}
void IDAT::modify_4(ImagePartial& image_partial) {
    int pos = 0;
    int n_scanline_bits = 8 + image_partial.width * image_partial.bit_depth * 2;
    int filter_type = 0;
    uint16_t gray_scale_value;
    uint16_t alpha_value;
    auto buffer_it = uncompressed_bytes.begin();
    // we need to read bits into pixels
    // calling set pixel
    // until end of stream
    while (pos < uncompressed_bytes.size() * 8) {
        if (!(pos % n_scanline_bits)) {
            // read filter type
            filter_type = slice_bits(pos, pos+8, buffer_it);
#if DEBUG
            std::cout << "Filter Type: " << filter_type << std::endl;
#endif
            pos+=8;
        } else {
            gray_scale_value = slice_bits(pos, pos+image_partial.bit_depth, buffer_it);
            pos += image_partial.bit_depth;
            alpha_value = slice_bits(pos, pos+image_partial.bit_depth, buffer_it);
            pos += image_partial.bit_depth;

            // TODO: handle 16 bit samples
            uint32_t pixel = 0;
            pixel |= alpha_value; 
            pixel |= (gray_scale_value << 8);
            pixel |= (gray_scale_value << 16);
            pixel |= (gray_scale_value << 24);
            image_partial.set_next_pixel(pixel);
        }
    }
}
void IDAT::modify_6(ImagePartial& image_partial) {
    int pos = 0;
    int n_scanline_bits = 8 + image_partial.width * image_partial.bit_depth * 4;
    int filter_type = 0;
    uint16_t red_value;
    uint16_t green_value;
    uint16_t blue_value;
    uint16_t alpha_value;
    auto buffer_it = uncompressed_bytes.begin();
    // we need to read bits into pixels
    // calling set pixel
    // until end of stream
    while (pos < uncompressed_bytes.size() * 8) {
        if (!(pos % n_scanline_bits)) {
            // read filter type
            filter_type = slice_bits(pos, pos+8, buffer_it);
#if DEBUG
            std::cout << "Filter Type: " << filter_type << std::endl;
#endif
            pos+=8;
        } else {
            red_value = slice_bits(pos, pos+image_partial.bit_depth, buffer_it);
            pos += image_partial.bit_depth;
            green_value = slice_bits(pos, pos+image_partial.bit_depth, buffer_it);
            pos += image_partial.bit_depth;
            blue_value = slice_bits(pos, pos+image_partial.bit_depth, buffer_it);
            pos += image_partial.bit_depth;
            alpha_value = slice_bits(pos, pos+image_partial.bit_depth, buffer_it);
            pos += image_partial.bit_depth;

            // TODO: handle 16 bit samples
            uint32_t pixel = 0;
            pixel |= (alpha_value << 24);
            pixel |= (blue_value  << 0);
            pixel |= (green_value << 8);
            pixel |= (red_value   << 16);
            image_partial.set_next_pixel(pixel);
        }
    }
}
