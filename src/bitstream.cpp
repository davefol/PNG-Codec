#include <cstdint>
#include <vector>
#include "bitstream.h"

// Creates a number out of a range of bits from
// a byte stream. Numbers do not have to fall
// on byte boundaries.
uint64_t slice_bits(int start, int end, std::vector<uint8_t>::iterator &buffer_it) {
    int byte_offset = start / 8;
    int bit_offset = start % 8;
    int power_of_2 = 1;
    uint64_t out = 0;

    // Reading the first bit.
    if (*(buffer_it + byte_offset) & (1 << bit_offset))
        out = 1;
    else
        out = 0;

    // Reading the rest of the bits
    for (int i = start+1; i < end; i++) {
        bit_offset++;
        if (bit_offset > 7) {
            bit_offset = 0;
            byte_offset++;
        }
        if (*(buffer_it + byte_offset) & (1 << bit_offset)) {
            out += 1 << power_of_2;
        }
        power_of_2++;
    }
    return out;
}
