#include <cstdint>
#include <vector>
#include <iostream>
#include "bitstream.h"

#define DEBUG 1

std::vector<uint8_t> inflate_stream(std::vector<uint8_t>::iterator &buffer_it) {
    int pos = 0;
    uint8_t bfinal = 0;

    // decoded output stream
    std::vector<uint8_t> out;

    // every time this gets full with an entire byte
    // we add it to out
    std::vector<bool> byte_buffer;

    do {
        // read block header from input stream
        bfinal = slice_bits(pos, pos+1, buffer_it);
        pos++;

        uint8_t btype = slice_bits(pos, pos+2, buffer_it);
        pos += 2;

#if DEBUG
        std::cout << "bfinal: " << (int)bfinal << ", btype: " << (int)btype << std::endl;
#endif

        // if stored with no compression
        if (btype == 0) {
            // skip any remanining bits in partially processed byte
            uint8_t remaining_bits = 8 - pos % 8;
            pos += remaining_bits;
            uint16_t len = slice_bits(pos, pos+16, buffer_it);
            pos += 16;
            uint16_t nlen = slice_bits(pos, pos+16, buffer_it);
            pos += 16;
            for (uint16_t i = 0; i < len; i++) {
                uint8_t copy_byte = slice_bits(pos, pos+16, buffer_it);
                pos += 16;
                out.push_back(copy_byte);
            }

        } else { // stored with compression
            // if stored with dynamic huffman codes
            if (btype == 2) {
                // read representation of code trees.
            }

            // loop until end of block code (256) is recognized
            //while (true) {
                // We are in Huffman code teritory!!!

            //}

        }

    } while (!bfinal);


    return out;
}
