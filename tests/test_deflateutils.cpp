#include <vector>
#include <catch2/catch.hpp>
#include <fstream>
#include <iostream>
#include "../include/deflateutils.h"
#include "../include/huffmanutils.h"

TEST_CASE("parse_length_code", "[huffman, deflate]") {
// REQUIRE (lvalue == rvalue)
    std::vector<SymbolLength> symbol_lengths {
        {0 , 7},
        {1 , 7},
        {2 , 7},
        {3 , 7},
        {4 , 7},
        {5 , 7},
        {6 , 4},
        {7 , 4},
        {8 , 3},
        {9 , 2},
        {10, 2},
        {11, 4},
        {12, 7},
        {13, 0},
        {14, 0},
        {16, 3},
        {17, 7},
        {18, 0},
    };

    std::vector<SymbolCode> huffman_codes = get_huffman_codes(symbol_lengths);
    std::shared_ptr<HuffmanNode> huffman_tree = std::make_shared<HuffmanNode>();
    for (auto it : huffman_codes) {
        huffman_tree->traverse_create(it.code.begin(), it.code.end(), it.symbol);
    }

    std::ifstream input("./test_data/code_length.input", std::ios::binary);
    std::vector<uint8_t> input_buffer(std::istreambuf_iterator<char>(input), {});

    std::ifstream output;
    output.open("./test_data/code_length.output");
    int n;
    output >> n;
    std::vector<int> expected(n);

    std::vector<int> code_lengths;
    auto input_buffer_it = input_buffer.begin();
    int pos = 0;
    while (code_lengths.size() < expected.size())
        pos = parse_length_code(huffman_tree, pos, input_buffer_it, code_lengths);

    REQUIRE(code_lengths == expected);
}
