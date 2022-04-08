#include <iostream>
#include <iomanip>
#include <cstdint>
#include <vector>
#include "IDAT.h"
#include "zlibutils.h"
#include "dumputils.h"

#define DEBUG 0


IDAT::IDAT(uint32_t _size, std::string _name, std::vector<uint8_t>::iterator &buffer_it) {
    size = _size;
    name = _name;
    
    uncompressed_bytes = uncompress_zlib_stream(size, buffer_it);
    
    dump_bytes("smile.raw", uncompressed_bytes.begin(), uncompressed_bytes.end());

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
    for (auto raw_byte : uncompressed_bytes) {
        image_partial.set_next_pixel(raw_byte);
    }
}
