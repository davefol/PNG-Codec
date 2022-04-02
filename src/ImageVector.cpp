#include <cstdint>
#include <vector>
#include "ImageVector.h"

ImageVector::ImageVector(int height, int width) {
    data = std::vector<std::vector<uint32_t>>(height);
    for (auto &it : data)
        it.resize(width);
}
