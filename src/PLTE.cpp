#include <cstdint>
#include <vector>
#include <iostream>
#include "consume.h"
#include "PLTE.h"

PLTE::PLTE(
        uint32_t _size,
        std::string _name,
        std::vector<uint8_t>::iterator& buffer_it
) {
    size = _size;
    name = _name;

    int n_palette_entries = _size / 3;
    for (int i = 0; i < n_palette_entries; i++) {
        uint32_t color = 0;
        color |= (0xFF << 24); // alpha value is most sig byte
        color |= (consume_uint8_t(buffer_it) << (8 * 2)); // red
        color |= (consume_uint8_t(buffer_it) << (8 * 1)); // green
        color |= (consume_uint8_t(buffer_it) << (8 * 0)); // blue
        palette.push_back(color);
    }
    advance(buffer_it, 4); // CRC
};

void PLTE::print(){
    std::cout << "PLTE:" << std::endl;
    for (int i = 0; i < palette.size(); i++) {
        std::cout << "(" << i << "): " << palette[i] << std::endl;
    }
}
void PLTE::modify(ImagePartial& image_partial) {
    image_partial.palette = palette;
}
