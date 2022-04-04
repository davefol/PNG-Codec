#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <iterator>
#include "UnknownChunk.h"
#include "consume.h"

UnknownChunk::UnknownChunk(uint32_t _size, std::string _name, std::vector<uint8_t>::iterator &buffer_it) {
    size = _size;
    name = _name; 
    advance(buffer_it, size);
    crc = consume_uint32_t(buffer_it);
}

void UnknownChunk::print() {
    std::cout << "UnknownChunk(size=" << size << ", name=" << name << ")" << std::endl;
}

void UnknownChunk::modify(ImagePartial& image_partial) {

}
