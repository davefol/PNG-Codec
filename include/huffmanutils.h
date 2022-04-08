#include <cstdint>
#include <vector>

#pragma once

typedef int HuffmanSymbol;

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

typedef std::shared_ptr<HuffmanNode> HuffmanTree;

struct SymbolLength {
    HuffmanSymbol symbol;
    int length;
};

struct SymbolCode {
    HuffmanSymbol symbol;
    std::vector<bool> code;
};

std::vector<SymbolCode> get_huffman_codes(std::vector<SymbolLength> symbol_lengths);

std::vector<SymbolCode> get_deflate_literal_length_huffman_codes();
HuffmanTree get_deflate_literal_length_huffman_node();


std::vector<SymbolCode> get_deflate_dist_huffman_codes();
HuffmanTree get_deflate_dist_huffman_node();
