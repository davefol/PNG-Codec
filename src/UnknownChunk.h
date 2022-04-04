#include <string>
#include <vector>
#include <cstdint>
#include <iterator>
#include "Chunk.h"

#pragma once

class UnknownChunk : public Chunk {
    uint32_t crc;

    public:
    UnknownChunk(uint32_t _size, std::string _name, std::vector<uint8_t>::iterator &buffer_it);
    void print();
    void modify(ImagePartial& image_partial);
};
