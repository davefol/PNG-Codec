#include <vector>
#include <cstdint>
#include "consume.h"

std::vector<uint8_t> inflate (std::vector<uint8_t> in) {
    std::vector<uint8_t> out;

    std::vector<uint8_t>::iterator buffer_it = in.begin();

    uint8_t compression_method = consume_uint8_t(buffer_it);

    
    return out;
}
