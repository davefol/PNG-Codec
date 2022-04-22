#include <iostream>
#include "IHDR.h"
#include "consume.h"
#include "ImagePartial.h"

#define DEBUG 1

struct Adam7PassDescription {
    int x_init;
    int y_init;
    int x_inc;
    int y_inc;
};

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
   image_partial.image_data.resize(height, std::vector<std::vector<uint16_t>>(width));
   
   if (interlace_method == 0) {
        std::vector<std::vector<std::vector<PixelPos>>> passes (1);
        std::vector<std::vector<PixelPos>> pass (height);
#if DEBUG
        std::cout << "N scanlines: " << pass.size() << std::endl;
#endif
        for (int y = 0; y < height; y++) {
            std::vector<PixelPos> scanline (width);
            for (int x = 0; x < width; x++) {
                scanline[x] = {y, x};
            }
            pass[y] = scanline;
        }
        passes[0] = pass;
        image_partial.passes = passes;

   }

   // set up the order for Adam7 Interlace
   //      1 6 4 6 2 6 4 6
   //      7 7 7 7 7 7 7 7
   //      5 6 5 6 5 6 5 6
   //      7 7 7 7 7 7 7 7
   //      3 6 4 6 3 6 4 6
   //      7 7 7 7 7 7 7 7
   //      5 6 5 6 5 6 5 6
   //      7 7 7 7 7 7 7 7
   // vector of passes, 7 passes
   // each pass contains scalines (the start of the scanline helps find filter byte)
   // each scaline contains x,y position
   else if (interlace_method == 1) {
        Adam7PassDescription pass_descriptions[7] = {
            {0, 0, 8, 8}, // 1
            {4, 0, 8, 8}, // 2
            {0, 4, 4, 8}, // 3
            {2, 0, 4, 4}, // 4
            {0, 2, 2, 4}, // 5
            {1, 0, 2, 2}, // 6
            {0, 1, 1, 2}, // 7
        };
        std::vector<std::vector<std::vector<PixelPos>>> passes;
        int x;
        int y;

        for (int i = 0; i < 7; i++) {
            std::vector<std::vector<PixelPos>> pass;
            y = pass_descriptions[i].y_init;
            while(y < height) {
                std::vector<PixelPos> scanline;
                x = pass_descriptions[i].x_init;
                while (x < width) {
                    scanline.push_back({y, x});
                    x+= pass_descriptions[i].x_inc;
                }
                // scanline finished, go to next
                pass.push_back(scanline);
                y+= pass_descriptions[i].y_inc;
            }
            passes.push_back(pass);
        }
        image_partial.passes = passes;
    }
}
