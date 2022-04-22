#include "ImagePartial.h"
#include "bitstream.h"
#include <iostream>

#define DEBUG 1

uint32_t l1dist(int a, int b) {
    return a > b ? a - b : b - a;
}


std::vector<uint16_t> ImagePartial::get_pixel_at(int si, int sxi) {
    PixelPos pp = passes[pass_index][si][sxi];
    return image_data[pp.height][pp.width];
}

ImagePartial::ImagePartial() {
    width = -1;
    height = -1;
    bit_depth = -1;
    color_type = -1;
    compression_method = -1;
    filler_method = -1;
    interlace_method = -1;
    pass_index = 0;
    scanline_index = 0;
    scanline_x_index = 0;
    next_pixel_pos = {0, 0};
    std::vector<std::vector<std::vector<uint16_t>>> image_data;
}

void ImagePartial::set_next_pixel(std::vector<uint16_t> pixel) {
    std::cout << "Setting " << next_pixel_pos.width << " " << next_pixel_pos.height << std::endl;
    image_data[next_pixel_pos.height][next_pixel_pos.width] = pixel;
    advance();
}

void ImagePartial::advance() {
    scanline_x_index++;
    // go to next scaline
    if (scanline_x_index >= passes[pass_index][scanline_index].size()) {
        scanline_x_index = 0;
        scanline_index++;
    }
    
    // go to next pass
    if (scanline_index >= passes[pass_index].size()) {
        scanline_index = 0;
        pass_index++;
    }


    if (pass_index < passes.size() && scanline_index < passes[pass_index].size() && scanline_x_index < passes[pass_index][scanline_index].size()){
        next_pixel_pos = passes[pass_index][scanline_index][scanline_x_index];
    }
}

void ImagePartial::generate_image() {
    switch (color_type) {
        case 0:
            modify_0();
            break;
        case 2:
            modify_2();
            break;
        case 3:
            modify_3();
            break;
        case 4:
            modify_4();
            break;
        case 6:
            modify_6();
            break;
    }
}
void ImagePartial::modify_0() {
    // TODO Handle modify_0 which involves filtering more than
    // one pixel in a single byte (sometimes)
    int pos = 0;
    int n_scanline_bits = 8 + width * bit_depth;
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
            gray_scale_value = slice_bits(pos, pos+bit_depth, buffer_it);
            pos += bit_depth;

            // TODO: handle 16 bit samples
            uint32_t pixel = 0;
            pixel |= (0xFF << 24); // full opacity
            pixel |= (gray_scale_value << 0);
            pixel |= (gray_scale_value << 8);
            pixel |= (gray_scale_value << 16);
            //image_partial.set_next_pixel(pixel);
        }
    }
}
void ImagePartial::modify_2() {
    int pos = 0;
    int n_scanline_bits = 8 + (width * (bit_depth * 3));
    int filter_type = 0;
    auto buffer_it = uncompressed_bytes.begin();
    int x = 0;
    int y = -1;
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
            y++;
            x=0;
        } else {
            uint16_t red_value = slice_bits(pos, pos+bit_depth, buffer_it);
            pos += bit_depth;
            uint16_t green_value = slice_bits(pos, pos+bit_depth, buffer_it);
            pos += bit_depth;
            uint16_t blue_value = slice_bits(pos, pos+bit_depth, buffer_it);
            pos += bit_depth;

            std::vector<uint16_t> pixel(4, 0);
            pixel[3] = 65535;
            pixel[2] = red_value;
            pixel[1] = green_value;
            pixel[0] = blue_value;

            if (filter_type == 1 && x > 0) {
                for (int c = 0; c < 4; c++) {
                    pixel[c] = (pixel[c] + get_pixel_at(y, x-1)[c]) % 256;
                }
            } else if (filter_type == 2 && y > 0) {
                for (int c = 0; c < 4; c++) {
                    pixel[c] = (pixel[c] + get_pixel_at(y-1, x)[c]) % 256;
                }
            } else if (filter_type == 3) {
                for (int c = 0; c < 4; c++) {
                    uint16_t left = x > 0 ? get_pixel_at(y, x-1)[c] % 256 : (uint16_t)0;
                    uint16_t up   = y > 0 ? get_pixel_at(y-1, x)[c] % 256 : (uint16_t)0;
                    pixel[c] = (pixel[c] + ((left + up) / 2)) % 256;
                }
            } else if (filter_type == 4) {
                for (int ch = 0; ch < 4; ch++) {
                    uint32_t a, b, c;
                    a = x > 0 ? get_pixel_at(y, x-1)[ch]  : 0;
                    b = y > 0 ? get_pixel_at(y-1, x)[ch]  : 0;
                    c = (x > 0 && y > 0) ? get_pixel_at(y-1, x-1)[ch]  : 0;

                    int p = a + b - c;
                    uint32_t pa = l1dist(p, a);
                    uint32_t pb = l1dist(p, b);
                    uint32_t pc = l1dist(p, c);
                    if (pa <= pb && pa <= pc)
                        pixel[ch] = (pixel[ch] + a) % 256;
                    else if (pb <= pc)
                        pixel[ch] = (pixel[ch] + b) % 256;
                    else
                        pixel[ch] = (pixel[ch] + c) % 256;
                }
            } 

            set_next_pixel(pixel);
            x++;
        }
    }
}

void ImagePartial::modify_3() {
    int pos = 0;
    int filter_type = 0;
    uint8_t palette_index;
    auto buffer_it = uncompressed_bytes.begin();
    // we need to read bits into pixels
    // calling set pixel
    // until end of stream
    while (pos < uncompressed_bytes.size() * 8) {
        if (scanline_x_index == 0) {
            // read filter type
            filter_type = slice_bits(pos, pos+8, buffer_it);
#if DEBUG
            std::cout << "Filter Type: " << filter_type << std::endl;
#endif
            pos+=8;
        }
        palette_index = slice_bits(pos, pos+bit_depth, buffer_it);
        pos += bit_depth;

        uint32_t palette_value = palette[palette_index];
        std::vector<uint16_t> pixel(4, 0);
        pixel[3] = (palette_value >> (3 * 8)) & 0xFF;
        pixel[2] = (palette_value >> (2 * 8)) & 0xFF;
        pixel[1] = (palette_value >> (1 * 8)) & 0xFF;
        pixel[0] = (palette_value >> (0 * 8)) & 0xFF;
        int x = scanline_x_index;
        int y = scanline_index;

        if (filter_type == 1 && x > 0) {
            for (int c = 0; c < 4; c++) {
                pixel[c] = (pixel[c] + get_pixel_at(y, x-1)[c]) % 256;
            }
        } else if (filter_type == 2 && y > 0) {
            for (int c = 0; c < 4; c++) {
                pixel[c] = (pixel[c] + get_pixel_at(y-1, x)[c]) % 256;
            }
        } else if (filter_type == 3) {
            for (int c = 0; c < 4; c++) {
                uint16_t left = x > 0 ? get_pixel_at(y, x-1)[c] % 256 : (uint16_t)0;
                uint16_t up   = y > 0 ? get_pixel_at(y-1, x)[c] % 256 : (uint16_t)0;
                pixel[c] = (pixel[c] + ((left + up) / 2)) % 256;
            }
        } else if (filter_type == 4) {
            for (int ch = 0; ch < 4; ch++) {
                uint32_t a, b, c;
                a = x > 0 ? get_pixel_at(y, x-1)[ch]  : 0;
                b = y > 0 ? get_pixel_at(y-1, x)[ch]  : 0;
                c = (x > 0 && y > 0) ? get_pixel_at(y-1, x-1)[ch]  : 0;

                int p = a + b - c;
                uint32_t pa = l1dist(p, a);
                uint32_t pb = l1dist(p, b);
                uint32_t pc = l1dist(p, c);
                if (pa <= pb && pa <= pc)
                    pixel[ch] = (pixel[ch] + a) % 256;
                else if (pb <= pc)
                    pixel[ch] = (pixel[ch] + b) % 256;
                else
                    pixel[ch] = (pixel[ch] + c) % 256;
            }
        } 
        set_next_pixel(pixel);
        
    }

}
void ImagePartial::modify_4() {
    int pos = 0;
    int filter_type = 0;
    uint16_t gray_scale_value;
    uint16_t alpha_value;
    auto buffer_it = uncompressed_bytes.begin();
    // we need to read bits into pixels
    // calling set pixel
    // until end of stream
    while (pos < uncompressed_bytes.size() * 8) {
        if (scanline_x_index == 0) {
            // read filter type
            filter_type = slice_bits(pos, pos+8, buffer_it);
#if DEBUG
            std::cout << "Filter Type: " << filter_type << std::endl;
#endif
            pos+=8;
        }
        gray_scale_value = slice_bits(pos, pos+bit_depth, buffer_it);
        pos += bit_depth;
        alpha_value = slice_bits(pos, pos+bit_depth, buffer_it);
        pos += bit_depth;

        std::vector<uint16_t> pixel (4, 0);
        pixel[3] = alpha_value;
        pixel[2] = gray_scale_value;
        pixel[1] = gray_scale_value;
        pixel[0] = gray_scale_value;
        
        int x = scanline_x_index;
        int y = scanline_index;

        if (filter_type == 1 && x > 0) {
            for (int c = 0; c < 4; c++) {
                pixel[c] = (pixel[c] + get_pixel_at(y, x-1)[c]) % 256;
            }
        } else if (filter_type == 2 && y > 0) {
            for (int c = 0; c < 4; c++) {
                pixel[c] = (pixel[c] + get_pixel_at(y-1, x)[c]) % 256;
            }
        } else if (filter_type == 3) {
            for (int c = 0; c < 4; c++) {
                uint16_t left = x > 0 ? get_pixel_at(y, x-1)[c] % 256 : (uint16_t)0;
                uint16_t up   = y > 0 ? get_pixel_at(y-1, x)[c] % 256 : (uint16_t)0;
                pixel[c] = (pixel[c] + ((left + up) / 2)) % 256;
            }
        } else if (filter_type == 4) {
            for (int ch = 0; ch < 4; ch++) {
                uint32_t a, b, c;
                a = x > 0 ? get_pixel_at(y, x-1)[ch]  : 0;
                b = y > 0 ? get_pixel_at(y-1, x)[ch]  : 0;
                c = (x > 0 && y > 0) ? get_pixel_at(y-1, x-1)[ch]  : 0;

                int p = a + b - c;
                uint32_t pa = l1dist(p, a);
                uint32_t pb = l1dist(p, b);
                uint32_t pc = l1dist(p, c);
                if (pa <= pb && pa <= pc)
                    pixel[ch] = (pixel[ch] + a) % 256;
                else if (pb <= pc)
                    pixel[ch] = (pixel[ch] + b) % 256;
                else
                    pixel[ch] = (pixel[ch] + c) % 256;
            }
        } 
        set_next_pixel(pixel);
    }
}
void ImagePartial::modify_6() {
    int pos = 0;
    int filter_type = 0;
    uint16_t red_value;
    uint16_t green_value;
    uint16_t blue_value;
    uint16_t alpha_value;
    auto buffer_it = uncompressed_bytes.begin();
    // we need to read bits into pixels
    // calling set pixel
    // until end of stream
    int x, y;
    while (!(scanline_index == height-1 && scanline_x_index == width-1)) {
        x = scanline_x_index;
        y = scanline_index;
        if (scanline_x_index == 0) {

            // read filter type
            filter_type = slice_bits(pos, pos+8, buffer_it);
            pos+=8;
#if DEBUG
            std::cout << "[" << scanline_index << "]" << "Filter Type: " << filter_type << std::endl;
#endif
        }
        red_value = slice_bits(pos, pos+bit_depth, buffer_it);
        pos += bit_depth;
        green_value = slice_bits(pos, pos+bit_depth, buffer_it);
        pos += bit_depth;
        blue_value = slice_bits(pos, pos+bit_depth, buffer_it);
        pos += bit_depth;
        alpha_value = slice_bits(pos, pos+bit_depth, buffer_it);
        pos += bit_depth;

        std::vector<uint16_t> pixel (4, 0);
        pixel[3] = alpha_value;
        pixel[2] = red_value;
        pixel[1] = green_value;
        pixel[0] = blue_value;

        if (filter_type == 1 && x > 0) {
            for (int c = 0; c < 4; c++) {
                pixel[c] = (pixel[c] + get_pixel_at(y, x-1)[c]) % 256;
            }
        } else if (filter_type == 2 && y > 0) {
            for (int c = 0; c < 4; c++) {
                pixel[c] = (pixel[c] + get_pixel_at(y-1, x)[c]) % 256;
            }
        } else if (filter_type == 3) {
            for (int c = 0; c < 4; c++) {
                uint16_t left = x > 0 ? get_pixel_at(y, x-1)[c] % 256 : (uint16_t)0;
                uint16_t up   = y > 0 ? get_pixel_at(y-1, x)[c] % 256 : (uint16_t)0;
                pixel[c] = (pixel[c] + ((left + up) / 2)) % 256;
            }
        } else if (filter_type == 4) {
            for (int ch = 0; ch < 4; ch++) {
                uint32_t a, b, c;
                a = x > 0 ? get_pixel_at(y, x-1)[ch]  : 0;
                b = y > 0 ? get_pixel_at(y-1, x)[ch]  : 0;
                c = (x > 0 && y > 0) ? get_pixel_at(y-1, x-1)[ch]  : 0;

                int p = a + b - c;
                uint32_t pa = l1dist(p, a);
                uint32_t pb = l1dist(p, b);
                uint32_t pc = l1dist(p, c);
                if (pa <= pb && pa <= pc)
                    pixel[ch] = (pixel[ch] + a) % 256;
                else if (pb <= pc)
                    pixel[ch] = (pixel[ch] + b) % 256;
                else
                    pixel[ch] = (pixel[ch] + c) % 256;
            }
        } 
        set_next_pixel(pixel);
    }
}

std::vector<std::vector<std::vector<uint16_t>>> ImagePartial::get_image() {

    return image_data; 
}
