#include <cstdint>
#include <vector>
#include <map>

#include "huffmanutils.h"

HuffmanNode::HuffmanNode() {
    left = std::shared_ptr<HuffmanNode>(nullptr);
    right = std::shared_ptr<HuffmanNode>(nullptr);
    symbol = -1;
}

std::shared_ptr<HuffmanNode> HuffmanNode::traverse_once(bool direction) {
    if (direction == 0)
        return left;
    else
        return right;
}

void HuffmanNode::traverse_create(std::vector<bool>::iterator first, std::vector<bool>::iterator last, int symbol) {
    // Only one move left in our path
    // means that we have reached a leaf node

    if (first == (last - 1)) {
        if (*first == false) {
            // create a node for 'left' and set its symbol
            left = std::make_shared<HuffmanNode>();
            left->symbol = symbol;
            return;
        }
        if (*first == true) {
            // create a node for 'right' and set its symbol
            right = std::make_shared<HuffmanNode>();
            right->symbol = symbol;
            return;
        }
    }

    if (*first == false) {
        if (left == nullptr)
            left = std::make_shared<HuffmanNode>();
        left->traverse_create(first+1, last, symbol);
        return;
    } else {
        if (right == nullptr)
            right = std::make_shared<HuffmanNode>();
        right->traverse_create(first+1, last, symbol);
        return;
    }
}

std::vector<SymbolCode> get_huffman_codes(std::vector<SymbolLength> symbol_lengths) {
    std::map<int, int> bl_count;
    std::map<int, int> next_code;
    int max_length = 0;

    // Count the number of codes for each code length
    for (auto it : symbol_lengths) {
        bl_count[it.length]++;
        max_length = std::max(it.length, max_length);
    }

    // Find the numerical value of the smallest code
    // for each code length
    int code = 0;
    for (int bits = 1; bits <=max_length; bits++) {
        code = (code + bl_count[bits-1]) << 1;
        next_code[bits] = code;
    }

    // generate all of the symbol codes
    std::vector<SymbolCode> symbol_code;
    for (auto it: symbol_lengths) {
        if (it.length) {
            int int_code = next_code[it.length];
            // convert this to a correctly sized
            // vector of bools;
            std::vector<bool> add_code (it.length);
            for (int bit = 0; bit < it.length; bit++) {
                add_code[bit] = (bool)(int_code & (1 << bit));
            }
            std::reverse(add_code.begin(), add_code.end());
            symbol_code.push_back({
                it.symbol,
                add_code
            });
            next_code[it.length]++;
        }
    }
    return symbol_code;
}

std::vector<SymbolCode> get_deflate_huffman_codes() {
    std::vector<SymbolLength> symbol_length (288);
    for (int symbol = 0; symbol <= 143; symbol++) {
        symbol_length[symbol] = {
            symbol,
            8
        };
    }
    for (int symbol = 144; symbol <= 255; symbol++) {
        symbol_length[symbol] = {
            symbol,
            9
        };
    }
    for (int symbol = 256; symbol <= 279; symbol++) {
        symbol_length[symbol] = {
            symbol,
            7
        };
    }
    for (int symbol = 280; symbol <= 287; symbol++) {
        symbol_length[symbol] = {
            symbol,
            8
        };
    }
    return get_huffman_codes(symbol_length);
}

std::shared_ptr<HuffmanNode> get_deflate_huffman_node() {
    std::vector<SymbolCode> symbol_code = get_deflate_huffman_codes();
    std::shared_ptr<HuffmanNode> root = std::make_shared<HuffmanNode>();
    for (auto it : symbol_code) {
        root->traverse_create(it.code.begin(), it.code.end(), it.symbol);
    }
    return root;
}
