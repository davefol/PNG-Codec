#include <cstdint>
#include <string>
#include <vector>
#include "Chunk.h"

#pragma once

class IDAT : public Chunk {
    private:
        std::vector<uint8_t> uncompressed_bytes;

    public:
        IDAT(uint32_t _size, std::string _name, std::vector<uint8_t>::iterator &buffer_it);
        void print();
};
