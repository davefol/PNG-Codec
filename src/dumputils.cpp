#include <iostream>
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
