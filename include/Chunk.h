#include <cstdint>
#include <string>
#include "ImagePartial.h"

#pragma once

class Chunk {
    public:
        uint32_t size;
        std::string name;
        virtual void print() = 0;
        virtual ~Chunk() = default;
        virtual void modify(ImagePartial& image_partial) = 0;
};
