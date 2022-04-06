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
                uint8_t hlit = slice_bits(pos, pos+5, buffer_it);
                pos+= 5;
                int n_literal_length_codes = hlit + 257;
#if DEBUG
                std::cout << "n_literal_length_codes: " << n_literal_length_codes << std::endl;
#endif
                uint8_t hdist = slice_bits(pos, pos+5, buffer_it);
                pos+= 5;
                int n_dist_codes = hdist + 1;
#if DEBUG
                std::cout << "n_dist_codes: " << n_dist_codes << std::endl;
#endif
                uint8_t hclen = slice_bits(pos, pos+4, buffer_it);
                pos+= 4;
                int n_code_len_codes = hclen + 4;

#if DEBUG
                std::cout << "n_code_len_codes: " << n_code_len_codes << std::endl;
#endif
                // fill up our code length codes
                std::vector<int> code_length_alphabet = {16, 17, 18,
                  0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
                std::vector<SymbolLength> code_length_symbol_length (n_code_len_codes);

                for (int i = 0; i < n_code_len_codes; i++) {
                    code_length_symbol_length[i] = {
                        code_length_alphabet[i],
                        (int)slice_bits(pos, pos+3, buffer_it)
                    };
                    pos+= 3;
                }
                // Huffman lengths need to be lexograpgically sorted according to zlib spec.
                std::sort(code_length_symbol_length.begin(), code_length_symbol_length.end(), 
                    [](auto a, auto b) -> bool {
                        return a.symbol < b.symbol;
                    }
                );
#if DEBUG
                std::cout << "Code Length length" << std::endl;
                for (auto it : code_length_symbol_length) {
                    std::cout << it.symbol << " " << it.length << std::endl;
                }
#endif
                // For code lengths:
                //     Create a Huffman Binary Tree from the Huffman Codes 
                std::vector<SymbolCode> code_length_huffman_codes = get_huffman_codes(code_length_symbol_length);
                std::shared_ptr<HuffmanNode> code_length_huffman_tree = std::make_shared<HuffmanNode>();
                for (auto it : code_length_huffman_codes) {
                    code_length_huffman_tree->traverse_create(it.code.begin(), it.code.end(), it.symbol);
                }

                std::vector<int> literal_length_codes;
                std::shared_ptr<HuffmanNode> current_node = code_length_huffman_tree;
                // TODO: rename literal length codes to literal lengths (its no longer encoded)
                // This while loop breaks much to early
                while(literal_length_codes.size() < n_literal_length_codes) {
                    bool next_bit;
                    while (true) {
                        next_bit = (bool)slice_bits(pos, pos+1, buffer_it);
                        pos++;
                        current_node = current_node->traverse_once(next_bit);
                        if (current_node->symbol != -1) {
                            if (current_node->symbol < 16) {
                                literal_length_codes.push_back(current_node->symbol);
#if DEBUG
                                std::cout << "lens " << current_node->symbol << std::endl;
#endif
                            } else {
                                //   16: Copy the previous code length 3 - 6 times.
                                //       The next 2 bits indicate repeat length
                                //             (0 = 3, ... , 3 = 6)
                                //          Example:  Codes 8, 16 (+2 bits 11),
                                //                    16 (+2 bits 10) will expand to
                                //                    12 code lengths of 8 (1 + 6 + 5)
                                //   17: Repeat a code length of 0 for 3 - 10 times.
                                //       (3 bits of length)
                                //   18: Repeat a code length of 0 for 11 - 138 times
                                //       (7 bits of length)
                                if (current_node->symbol == 16) {
                                    uint8_t repeat_length = slice_bits(pos, pos+2, buffer_it) + 3;
                                    pos+=2;
                                    for (int i = 0; i < repeat_length; i++)
                                        literal_length_codes.push_back(literal_length_codes.back());
#if DEBUG
                                    std::cout << "repeats " << (int)repeat_length << std::endl;
#endif
                                } else if (current_node->symbol == 17) {
                                    uint8_t times_0 = slice_bits(pos, pos+3, buffer_it) + 3;
                                    pos+=3;
                                    for (int i = 0; i < times_0; i++) {
                                        literal_length_codes.push_back(0);
                                    }
#if DEBUG
                                    std::cout << "zeros " << (int)times_0 << std::endl;
#endif
                                } else if (current_node->symbol == 18) {
                                    uint8_t times_0 = slice_bits(pos, pos+7, buffer_it) + 3;
                                    pos+=7;
                                    for (int i = 0; i < times_0; i++) {
                                        literal_length_codes.push_back(0);
                                    }
#if DEBUG
                                    std::cout << "zeros " << (int)times_0 << std::endl;
#endif
                                }
                            }
                            current_node = code_length_huffman_tree;
                            break;
                        }
                    }
                }
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
