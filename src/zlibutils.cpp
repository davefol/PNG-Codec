#include <cstdint>
#include <vector>
#include <iostream>
#include <iomanip>
#include "consume.h"
#include "zlibutils.h"
#include "deflateutils.h"
#include "dumputils.h"

#define DEBUG 1


std::vector<uint8_t> uncompress_zlib_stream(int size, std::vector<uint8_t>::iterator &buffer_it) {
    uint8_t compression_method = consume_uint8_t(buffer_it);   
    uint8_t flags = consume_uint8_t(buffer_it);

#if DEBUG
    std::cout << "flags: " << std::setw(2) << std::setfill('0') << std::hex << (int)flags << std::dec << std::endl;
#endif

    // separate compression method into
    // compression method (always 8) and compression info
    // then compute LZ77 window
    uint8_t compression_method_inner = compression_method & 0xF;
    uint8_t compression_info = (compression_method & 0xF0) >> 4;
    uint32_t lz77_window_size = 1 << (compression_info + 8);

#if DEBUG
    std::cout << "compression_method_inner: " << (int)compression_method_inner << std::endl;
    std::cout << "compression_info: " << (int)compression_info << std::endl;
    std::cout << "lz77_window_size: " << (int)lz77_window_size << std::endl;
#endif

    // check check value
    // uint8_t fcheck = flags & 0x1F; // 5 least signficant bits of flags
    bool fdict = flags & (1 << 5); // bit 6 of flags
    // uint8_t flevel = (flags & 0xc0) >> 6; // not needed for decompression
    // bool check_valid = (compression_method * 256 + flags) % 31;

    // check if fdict has been set, which will reduce
    // the total size of our compressed data we need to read.

#if DEBUG
    std::cout << "fdict: " << fdict << std::endl;
#endif
    std::vector<uint8_t> out;
    if (fdict) {
        //uint32_t dictid = consume_uint32_t(buffer_it);
        consume_uint32_t(buffer_it);
        dump_bytes("data.gz", buffer_it, buffer_it+size-10);
        out = inflate_stream(buffer_it);
        advance(buffer_it, size - 10);
    } else {
        dump_bytes("data.gz", buffer_it, buffer_it+size-6);
        out = inflate_stream(buffer_it);
        advance(buffer_it, size - 6);
    }
    //dump_bytes("data.raw", out.begin(), out.end());
    

    uint32_t check_value = consume_uint32_t(buffer_it);
#if DEBUG
    std::cout << "check_value: " << (int)check_value << std::endl;
#endif
    return out;
}
