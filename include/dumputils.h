#include <string>
#include <vector>

#pragma once

void dump_bytes(std::string fname, std::vector<uint8_t>::iterator first, std::vector<uint8_t>::iterator last);
void dump_bytes(
    std::string fname,
    std::vector<std::vector<uint32_t>>::iterator first,
    std::vector<std::vector<uint32_t>>::iterator last
);
