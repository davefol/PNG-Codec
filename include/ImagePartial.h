#include <vector>
#include <optional>
#pragma once

struct PixelPos {
    int height;
    int width;
};

class ImagePartial {
    // Use std::optional
    public:
        int width;
        int height;
        int bit_depth;
        int color_type;
        int compression_method;
        int filler_method;
        int interlace_method;
        std::vector<uint8_t> uncompressed_bytes;

        std::vector<uint32_t> palette;

        PixelPos next_pixel_pos;
        std::vector<std::vector<std::vector<uint16_t>>> image_data;
        ImagePartial();
        void set_next_pixel(std::vector<uint16_t> pixel);
        void advance();
        std::vector<std::vector<std::vector<uint16_t>>> get_image();
        void generate_image();
        void modify_0();
        void modify_2();
        void modify_3();
        void modify_4();
        void modify_6();
};

