#include <cstdint>
#include <vector>
#include "huffmanutils.h"

#pragma once

std::vector<uint8_t> inflate_stream(std::vector<uint8_t>::iterator &buffer_it);

int parse_length_code(const HuffmanTree& huffman_tree, const int& start, const std::vector<uint8_t>::iterator& buffer_it, std::vector<int>& code_lengths);

int huffman_tree_from_length(int n, int pos, const std::vector<uint8_t>& buffer_it, const HuffmanTree& code_length_tree, HuffmanTree out);

//                  Extra           Extra               Extra
//             Code Bits Dist  Code Bits   Dist     Code Bits Distance
//             ---- ---- ----  ---- ----  ------    ---- ---- --------
//               0   0    1     10   4     33-48    20    9   1025-1536
//               1   0    2     11   4     49-64    21    9   1537-2048
//               2   0    3     12   5     65-96    22   10   2049-3072
//               3   0    4     13   5     97-128   23   10   3073-4096
//               4   1   5,6    14   6    129-192   24   11   4097-6144
//               5   1   7,8    15   6    193-256   25   11   6145-8192
//               6   2   9-12   16   7    257-384   26   12  8193-12288
//               7   2  13-16   17   7    385-512   27   12 12289-16384
//               8   3  17-24   18   8    513-768   28   13 16385-24576
//               9   3  25-32   19   8   769-1024   29   13 24577-32768

struct DistInfo {
    int distance;
    int extra_bits;
};

static constexpr DistInfo dist_info[] = {
    {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 1}, {7, 1},
    {9, 2}, {13, 2}, {17, 3}, {25, 3}, {33, 4}, {49, 4},
    {65, 5}, {97, 5}, {129, 6}, {193, 6}, {257, 7},
    {385, 7}, {513, 8}, {769, 8}, {1025, 9}, {1537, 9},
    {2049, 10}, {3073, 10}, {4097, 11}, {6145, 11},
    {8193, 12}, {12289, 12}, {16385, 13}, {24577, 13}
};



//                 Extra               Extra               Extra
//            Code Bits Length(s) Code Bits Lengths   Code Bits Length(s)
//            ---- ---- ------     ---- ---- -------   ---- ---- -------
//             257   0     3       267   1   15,16     277   4   67-82
//             258   0     4       268   1   17,18     278   4   83-98
//             259   0     5       269   2   19-22     279   4   99-114
//             260   0     6       270   2   23-26     280   4  115-130
//             261   0     7       271   2   27-30     281   5  131-162
//             262   0     8       272   2   31-34     282   5  163-194
//             263   0     9       273   3   35-42     283   5  195-226
//             264   0    10       274   3   43-50     284   5  227-257
//             265   1  11,12      275   3   51-58     285   0    258
//             266   1  13,14      276   3   59-66

struct LengthInfo {
    int length;
    int extra_bits;
};

static constexpr LengthInfo length_info[] = {
    { 3, 0 }, // 257
    { 4, 0 },
    { 5, 0 },
    { 6, 0 },
    { 7, 0 },
    { 8, 0 },
    { 9, 0 },
    { 10, 0 },
    { 11, 1 },
    { 13, 1 },
    { 15, 1 },
    { 17, 1 },
    { 19, 2 },
    { 23, 2 },
    { 27, 2 },
    { 31, 2 },
    { 35, 3 },
    { 43, 3 },
    { 51, 3 },
    { 59, 3 },
    { 67, 4 },
    { 83, 4 },
    { 99, 4 },
    { 115, 4 },
    { 131, 5 },
    { 163, 5 },
    { 195, 5 },
    { 227, 5 },
    { 258, 0 },
};

