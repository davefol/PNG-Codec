#include <cstdint>
#include <vector>

#pragma once

class HuffmanNode {
    private:
        std::shared_ptr<HuffmanNode> left;
        std::shared_ptr<HuffmanNode> right;

    public:
        int symbol;
        HuffmanNode();
        std::shared_ptr<HuffmanNode> traverse_once(bool direction);
        void traverse_create(std::vector<bool>::iterator first, std::vector<bool>::iterator last, int symbol);
};

struct SymbolLength {
    int symbol;
    int length;
};

struct SymbolCode {
    int symbol;
    std::vector<bool> code;
};

std::vector<SymbolCode> get_huffman_codes(std::vector<SymbolLength> symbol_lengths);

std::vector<SymbolCode> get_deflate_huffman_codes();
std::shared_ptr<HuffmanNode> get_deflate_huffman_node();
