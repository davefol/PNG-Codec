#include <cstdint>
#include <vector>

#pragma once

uint64_t slice_bits(int start, int end, std::vector<uint8_t>::iterator &buffer_it);
