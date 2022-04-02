#include <cstdint>
#include <vector>
#include <iterator>

#pragma once

uint32_t consume_uint32_t(std::vector<uint8_t>::iterator &buffer_it);
uint8_t consume_uint8_t(std::vector<uint8_t>::iterator &buffer_it);
