#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include "IDAT.h"
#include "zlibutils.h"
#include "dumputils.h"
#include "bitstream.h"

#define DEBUG 1



IDAT::IDAT(uint32_t _size, std::string _name, std::vector<uint8_t>::iterator &buffer_it) {
    size = _size;
    name = _name;
    
    //dump_bytes("compressed_bytes.gz", buffer_it, buffer_it+size);
    uncompressed_bytes = uncompress_zlib_stream(size, buffer_it);
#if DEBUG
    std::cout << "Got uncompressed bytes succesfully" << std::endl;
#endif
    
    //dump_bytes("uncompressed_bytes.raw", uncompressed_bytes.begin(), uncompressed_bytes.end());

    advance(buffer_it, 4); // CRC
}

void IDAT::print() {
};

void IDAT::modify(ImagePartial& image_partial) {
    //dump_bytes("uncompressed.raw", uncompressed_bytes.begin(), uncompressed_bytes.end());
    image_partial.uncompressed_bytes.insert(
            image_partial.uncompressed_bytes.end(),
            uncompressed_bytes.begin(),
            uncompressed_bytes.end()
    );
}

