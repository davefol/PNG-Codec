#include <cstdint>
#include <vector>

#pragma once

class ImageVector {
    public:
        ImageVector(int height, int width);
        std::vector<std::vector<uint32_t>> data;
};
