#include <cstdint>
#include <vector>
#include <iostream>
#include <iomanip>
#include "deflateutils.h"
#include "bitstream.h"
#include "huffmanutils.h"
#include "dumputils.h"

#define DEBUG 1

int huffman_tree_from_length(
        int n,
        int pos,
        const std::vector<uint8_t>::iterator& buffer_it,
        const HuffmanTree& code_length_huffman_tree,
        HuffmanTree out
) {
    std::vector<int> code_lengths;
    while(code_lengths.size() < n) {
        pos = parse_length_code(code_length_huffman_tree, pos, buffer_it, code_lengths);
    }
    std::cout << "parsed all length codes" << std::endl;
    std::vector<SymbolLength> codes_with_symbols;
    for (int symbol = 0; symbol < n; symbol++) {
        codes_with_symbols.push_back({
           symbol, code_lengths[symbol] 
        });
    }
    std::vector<SymbolCode> huffman_code = get_huffman_codes(codes_with_symbols);
#if DEBUG
    std::cout << "Initializing huffman tree" << std::endl;
#endif 
    for (auto it : huffman_code) {
        out->traverse_create(it.code.begin(), it.code.end(), it.symbol);
    }
#if DEBUG
    std::cout << "Finished initializing huffman tree" << std::endl;
#endif
    return pos;
}


int parse_length_code(
    const HuffmanTree& huffman_tree,
    const int& start,
    const std::vector<uint8_t>::iterator& buffer_it,
    std::vector<int>& code_lengths
) {
    
    bool next_bit;
    int pos = start;
    auto current_node = huffman_tree;
    // we advance each time we reach a leaf node in our Huffman tree
    while (true) {
        next_bit = (bool)slice_bits(pos, pos+1, buffer_it);
        pos++;
        current_node = current_node->traverse_once(next_bit);
        std::cout << (current_node == nullptr);
        if (current_node->symbol != -1) {
            if (current_node->symbol < 16) {
                code_lengths.push_back(current_node->symbol);
#if DEBUG
                std::cout << "lens " << current_node->symbol << std::endl;
#endif
                return pos;
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
                        code_lengths.push_back(code_lengths.back());
#if DEBUG
                    std::cout << "repeats " << (int)repeat_length << std::endl;
#endif
                    return pos;
                } else if (current_node->symbol == 17) {
                    uint8_t times_0 = slice_bits(pos, pos+3, buffer_it) + 3;
                    pos+=3;
                    for (int i = 0; i < times_0; i++) {
                        code_lengths.push_back(0);
                    }
#if DEBUG
                    std::cout << "zeros " << (int)times_0 << std::endl;
#endif
                    return pos;
                } else if (current_node->symbol == 18) {
                    uint8_t times_0 = slice_bits(pos, pos+7, buffer_it) + 11;
                    pos+=7;
                    for (int i = 0; i < times_0; i++) {
                        code_lengths.push_back(0);
                    }
#if DEBUG
                    std::cout << "zeros " << (int)times_0 << std::endl;
#endif
                    return pos;
                }
            }
        }
    }
}

std::vector<uint8_t> inflate_stream(std::vector<uint8_t>::iterator &buffer_it) {
    int pos = 0;
    uint8_t bfinal = 0;

    // decoded output stream
    std::vector<uint8_t> out;

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
#if DEBUG
            std::cout << "pos at start of uncompressed chunk: " << (int)pos << std::endl;
#endif
            uint8_t remaining_bits = 8 - (pos % 8);
            pos += remaining_bits;
#if DEBUG
            std::cout << (int)remaining_bits << std::endl;
#endif

            uint16_t len = slice_bits(pos, pos+16, buffer_it);
            pos += 16;
            uint16_t nlen = slice_bits(pos, pos+16, buffer_it);
            pos += 16;

#if DEBUG
            std::cout << "remaining bits: " << (int)remaining_bits << std::endl;
            std::cout << "len: " << int(len) << std::endl;
            std::cout << "~nlen: " << int(~nlen) << std::endl;
#endif
            for (uint16_t i = 0; i < len; i++) {
                uint8_t copy_byte = slice_bits(pos, pos+8, buffer_it);
                pos += 8;
                out.push_back(copy_byte);
            }

            // stored with compression
        } else {
            HuffmanTree literal_length_huffman_tree = std::make_shared<HuffmanNode>();
            HuffmanTree dist_huffman_tree = std::make_shared<HuffmanNode>();
            // stored with fixed Huffman codes.
            if (btype == 1) {
                // load fixed huffman code tree
                literal_length_huffman_tree = get_deflate_literal_length_huffman_node();
                dist_huffman_tree = get_deflate_dist_huffman_node();
            }
            // if stored with dynamic Huffman codes (btype = 2)
            else {
                // read representation of code trees.
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
#if DEBUG
                std::cout << "Code Length length" << std::endl;
                for (auto it : code_length_symbol_length) {
                    if (it.length > 0)
                        std::cout << "code " << it.symbol << " " << it.length << std::endl;
                }
#endif
                // Huffman lengths need to be lexograpgically sorted according to zlib spec.
                std::sort(code_length_symbol_length.begin(), code_length_symbol_length.end(), 
                    [](auto a, auto b) -> bool {
                        return a.symbol < b.symbol;
                    }
                );
                // For code lengths:
                //     Create a Huffman Binary Tree from the Huffman Codes 
                std::vector<SymbolCode> code_length_huffman_codes = get_huffman_codes(code_length_symbol_length);
                HuffmanTree code_length_huffman_tree = std::make_shared<HuffmanNode>();
                for (auto it : code_length_huffman_codes) {
                    code_length_huffman_tree->traverse_create(it.code.begin(), it.code.end(), it.symbol);
                }

#if DEBUG
                std::cout << "Huffman codes" << std::endl;
                for (auto it : code_length_huffman_codes) {
                    std::cout << it.symbol << " ";
                    for (auto bit: it.code) {
                        std::cout << (int)bit;
                    }
                    std::cout << std::endl;
                }
#endif
                pos = huffman_tree_from_length(
                    n_literal_length_codes,
                    pos, 
                    buffer_it,
                    code_length_huffman_tree,
                    literal_length_huffman_tree
                );

                pos = huffman_tree_from_length(
                    n_dist_codes,
                    pos,
                    buffer_it,
                    code_length_huffman_tree,
                    dist_huffman_tree
                );


                // at this point the literal/length and distance huffman trees are fully initialized.
                std::cout << "*************" << std::endl;
            }

            // loop until end of block code (256) is recognized
#if DEBUG
            std::cout << "finished parsing Huffman Trees" << std::endl;
#endif
            HuffmanTree current_node = literal_length_huffman_tree;
            int decoded_symbol;
            bool next_bit;
            while (true) {
                // We are in Huffman code teritory!!!
                // decode literal/length value from input stream
                current_node = literal_length_huffman_tree;
                while (true) {
                    // if we have decoded a symbol
                    next_bit = (bool)slice_bits(pos, pos+1, buffer_it);
                    pos++;
#if DEBUG
                    if (btype == 1)
                        std::cout << (int)next_bit;
#endif
                    current_node = current_node->traverse_once(next_bit);
                    if (current_node->symbol != -1) {
                        // if value is less than 256 copy to output stream
                        decoded_symbol = current_node->symbol;
                        current_node = literal_length_huffman_tree;
#if DEBUG
                        if (btype == 1)
                            std::cout << " " << std::endl;
#endif
                        break;
                    }
                }
                // if literal copy to output stream
                if (decoded_symbol < 256) {
#if DEBUG
                    std::cout << "literal " << decoded_symbol << std::endl;
#endif
                    out.push_back((uint8_t)decoded_symbol);
                } else if (decoded_symbol == 256) {
#if DEBUG
                    std::cout << "END OF DEFLATE CHUNK" << std::endl;
#endif
                    break;
                } else {
                    // Read extra bits for length code
                    int length_code = decoded_symbol - 257;
                    int length = length_info[length_code].length;
                    if (length_info[length_code].extra_bits) {
                        length += slice_bits(pos, pos+length_info[length_code].extra_bits, buffer_it);
                        pos += length_info[length_code].extra_bits;
                    }

                    // decode distance bits
                    int distance;
                    current_node = dist_huffman_tree;
                    while (true) {
                        next_bit = (bool)slice_bits(pos, pos+1, buffer_it);
                        pos++;
                        current_node = current_node->traverse_once(next_bit);
                        if (current_node->symbol != -1) {
                            // read the next n bits 
                            auto distance_info = dist_info[current_node->symbol];
                            int offset;
                            if (distance_info.extra_bits)
                                offset = slice_bits(pos, pos+distance_info.extra_bits, buffer_it);
                            else
                                offset = 0;
                            pos+=distance_info.extra_bits;
                            distance = distance_info.distance + offset;
                            break;
                        }
                    }

                    // now we have length, distance
                    // we go back distance bytes
                    // and repeat that byte length times
                    // at the end of our output stream
                    //
                    // Note also that the referenced string may overlap the current
                    // position; for example, if the last 2 bytes decoded have values
                    // X and Y, a string reference with <length = 5, distance = 2>
                    // adds X,Y,X,Y,X to the output stream.
                    
                    int pick_point = std::distance(out.begin(), out.end() - distance);
                    for (int i = 0; i < length; i++) {
                        out.push_back(out[pick_point]);
                        pick_point++;
                    }
                    //out.insert(out.end(), out.end()-distance, out.end()-distance+length);
#if DEBUG
                    std::cout << "match " << length << " " << distance << std::endl;
#endif
                }
            }

        }

    } while (!bfinal);

    return out;
}
