#include <cstdint>
#include <string>
#include <vector>
#include "Chunk.h"

#pragma once

class IDAT : public Chunk {
    private:
        std::vector<uint8_t> uncompressed_bytes;
        void modify_0(ImagePartial& image_partial);
        void modify_2(ImagePartial& image_partial);
        void modify_3(ImagePartial& image_partial);
        void modify_4(ImagePartial& image_partial);
        void modify_6(ImagePartial& image_partial);

    public:
        IDAT(uint32_t _size, std::string _name, std::vector<uint8_t>::iterator &buffer_it);
        void print();
        void modify(ImagePartial& image_partial);
};
