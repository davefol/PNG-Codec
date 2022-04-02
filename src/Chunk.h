#include <cstdint>
#include <string>

#pragma once

class Chunk {
    public:
        uint32_t size;
        std::string name;
        virtual void print() = 0;
        virtual ~Chunk() = default;

};
