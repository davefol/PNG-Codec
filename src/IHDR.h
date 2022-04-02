#include <cstdint>
#include <string>
#include <vector>
#include "Chunk.h"

#pragma once

class IHDR : public Chunk {
    uint32_t width;             // 4 bytes
    uint32_t height;            // 4 btyes
    uint8_t bit_depth;          // 1 byte
    uint8_t color_type;         // 1 byte
    uint8_t compression_method; // 1 byte
    uint8_t filler_method;      // 1 byte
    uint8_t interlace_method;   // 1 byte

    public:
        IHDR(uint32_t _size, std::string _name, std::vector<uint8_t>::iterator &buffer_it);
        void print();
};
