#include <cstdint>
#include <vector>
#include <iostream>
#include <iomanip>
#include "bitstream.h"
#include "huffmanutils.h"

#define DEBUG 1

std::vector<uint8_t> inflate_stream(std::vector<uint8_t>::iterator &buffer_it) {
    int pos = 0;
    uint8_t bfinal = 0;

    // decoded output stream
    std::vector<uint8_t> out;

    // every time this gets full with an entire byte
    // we add it to out
    std::vector<bool> byte_buffer;

    do {
        // read block header from input stream
        bfinal = slice_bits(pos, pos+1, buffer_it);
        pos++;

        uint8_t btype = slice_bits(pos, pos+2, buffer_it);
        pos += 2;

#if DEBUG
        std::cout << "bfinal: " << (int)bfinal << ", btype: " << (int)btype << std::endl;
#endif

        // if stored with no compression
        if (btype == 0) {
            // skip any remanining bits in partially processed byte
            uint8_t remaining_bits = 8 - pos % 8;
            pos += remaining_bits;
            uint16_t len = slice_bits(pos, pos+16, buffer_it);
            pos += 16;
            uint16_t nlen = slice_bits(pos, pos+16, buffer_it);
            pos += 16;
            for (uint16_t i = 0; i < len; i++) {
                uint8_t copy_byte = slice_bits(pos, pos+16, buffer_it);
                pos += 16;
                out.push_back(copy_byte);
            }

            // stored with compression
        } else {
            std::shared_ptr<HuffmanNode> huffman_tree;
            // stored with fixed Huffman codes.
            if (btype == 1) {
                // load fixed huffman code tree
                huffman_tree = get_deflate_huffman_node();
            }
            // if stored with dynamic Huffman codes (btype = 2)
            else {
                // read representation of code trees.
                // TODO
            }

            // loop until end of block code (256) is recognized
            std::shared_ptr<HuffmanNode> current_node = huffman_tree;
            int decoded_symbol;
            bool next_bit;
            while (true) {
                // We are in Huffman code teritory!!!
                    // decode literal/length value from input stream
                    while (true) {
                        // if we have decoded a symbol
                        next_bit = (bool)slice_bits(pos, pos+1, buffer_it);
#if DEBUG >= 2
                        std::cout << "next_bit :" << (int)next_bit << std::endl;
#endif
                        current_node = current_node->traverse_once(next_bit);
#if DEBUG >= 2
                        std::cout << "Current Node: " << current_node->symbol << std::endl;
#endif
                        pos++;
                        if (current_node->symbol != -1) {
                            // if value is less than 256 copy to output stream
                            decoded_symbol = current_node->symbol;
                            current_node = huffman_tree;
                            break;
                    }
                }
                // if literal copy to output stream
                if (decoded_symbol < 256) {
#if DEBUG
                    std::cout << "decoded literal: " << std::setw(2) << std::setfill('0') << std::hex << decoded_symbol << std::dec << std::endl;
#endif
                    out.push_back((uint8_t)decoded_symbol);
                } else if (decoded_symbol == 256) {
                    break;
                } else {
                    // decode distance bits (fixed 5 bits)
                }
            }

        }

    } while (!bfinal);

    return out;
}
