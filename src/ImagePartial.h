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
        PixelPos next_pixel_pos;
        std::vector<std::vector<uint32_t>> image_data;
        ImagePartial();
        void set_next_pixel(uint32_t pixel);
        void advance();
        std::vector<std::vector<uint32_t>> get_image();
};

