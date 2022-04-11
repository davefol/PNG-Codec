#include <iostream>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <string>
#include <vector>
#include <fstream>
#include "dumputils.h"

void dump_bytes(std::string fname, std::vector<uint8_t>::iterator first, std::vector<uint8_t>::iterator last) {
    std::cout << "DUMPING BYTES TO: " << fname << std::endl;
    std::ofstream wf(fname, std::ios::out | std::ios::binary);
    std::vector<uint8_t> vc;
    std::copy(first, last, back_inserter(vc));
    wf.write((char *)vc.data(), vc.size());
    wf.close();
    if (wf.bad()) {
        std::cout << "Bad Write!!!" << std::endl;
    }
}

void dump_bytes(
        std::string fname,
        std::vector<std::vector<uint32_t>>::iterator first,
        std::vector<std::vector<uint32_t>>::iterator last
) {
    std::cout << "DUMPING BYTES TO: " << fname << std::endl;
    std::ofstream wf(fname, std::ios::out | std::ios::binary);

    while(first != last) {
       for (auto pixel : *first) {
           std::cout << "(";
            for (int byte_pos = 0; byte_pos < 4; byte_pos++) {
                uint8_t channel = (pixel >> ((3 - byte_pos) * 8) & (0xFF));
                std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)channel << std::dec << " ";
            }
            std::cout << ")";
       } 
        std::cout << std::endl;
       first++;
    }
    if (wf.bad()) {
        std::cout << "Bad Write!!!" << std::endl;
    }
}
