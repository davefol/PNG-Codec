#include "Chunk.h"
#pragma once


class PLTE : public Chunk {
    private:
        std::vector<uint32_t> palette;

    public: 
        PLTE(
            uint32_t _size,
            std::string _name,
            std::vector<uint8_t>::iterator& buffer_it
        );
        void print();
        void modify(ImagePartial& image_partial);

};
