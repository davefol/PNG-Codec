#include <cstdint>
#include <vector>
#include "IDAT.h"
#include "zlibutils.h"


IDAT::IDAT(uint32_t _size, std::string _name, std::vector<uint8_t>::iterator &buffer_it) {
    size = _size;
    name = _name;
    
    uncompressed_bytes = uncompress_zlib_stream(size, buffer_it);
    advance(buffer_it, 4); // CRC

}

void IDAT::print() {
};

void IDAT::modify(ImagePartial& image_partial) {
    for (auto raw_byte : uncompressed_bytes) {
        image_partial.set_next_pixel(raw_byte);
    }
}
