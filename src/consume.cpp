#include <cstdint>
#include <vector>
#include <iterator>
#include "consume.h"


uint8_t consume_uint8_t(std::vector<uint8_t>::iterator &buffer_it) {
    uint8_t out = *buffer_it;
    advance(buffer_it, 1);
    return out;
}

uint32_t consume_uint32_t(std::vector<uint8_t>::iterator &buffer_it) {
    uint32_t out = 0;
    // 0000 0000 0000 0000 0000 0000 0000 0000
    // [buffer_it+0] [buffer_it+1] [buffer_it+2] [buffer_it+3]
    //
    // buffer_it + 0 is 5 so...
    // 0000 0101 0000 0000 0000 0000 0000 0000
    // which is just 5 left shifted 24 times (8 * 3)
    
    // decimal analogy
    // 1 2 3 4
    // stream: 1, 2, 3, 4
    // add 3 zeros to first digit (left shift 3) 1 0 0 0
    // add 2 zeros to second digit (left shift 2) 2 0 0
    // add 1 zero to third digit (left shift 1) 3 0
    // add 0 zeros to fourth digit (left shift 0) 4
    // add all of our shifted numbers 1000 + 200 + 30 + 4 = 1234
    // or we can think of this as or-ing our numbers
    // 1000
    // 0200
    // 1200
    // continue this process...
    out |= (uint8_t)(*(buffer_it + 0)) << (8 * 3); // 
    out |= (uint8_t)(*(buffer_it + 1)) << (8 * 2);
    out |= (uint8_t)(*(buffer_it + 2)) << (8 * 1);
    out |= (uint8_t)(*(buffer_it + 3)) << (8 * 0);
    advance(buffer_it, 4);
    return out;
}
