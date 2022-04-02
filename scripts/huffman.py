from collections import defaultdict
from functools import reduce
from typing import List, Tuple, Union

class Symbol:
    def __init__(self, value: int):
        self.value = value

    def __repr__(self):
        return self.value.__repr__()

    def __str__(self):
        return self.value.__str__()

    def traverse(self, _: str):
        return self.value

class UndefinedNode:
    """Defines an unconstructed region in a btree"""
    def traverse(self):
        raise ValueError("Undefined nodes are not traversable")

class BTreeNode:
    def __init__(self, left: Union["BTreeNode", Symbol, UndefinedNode], right: Union["BTreeNode", Symbol, UndefinedNode]):
        self.left = left
        self.right = right

    def traverse(self, path: str) -> Symbol:
        if path[0] == "0":
            return self.left.traverse(path[1:])
        if path[0] == "1":
            return self.right.traverse(path[1:])

    def traverse_create(self, path: str, symbol: Symbol):
        if len(path) == 1:
            if path[0] == "0":
                self.left = symbol
            if path[0] == "1":
                self.right = symbol
            return

        if path[0] == "0":
            if isinstance(self.left, UndefinedNode):
                self.left = BTreeNode(UndefinedNode(), UndefinedNode())
            self.left.traverse_create(path[1:], symbol)
        if path[0] == "1":
            if isinstance(self.right, UndefinedNode):
                self.right = BTreeNode(UndefinedNode(), UndefinedNode())
            self.right.traverse_create(path[1:], symbol)

    def traverse_once(self, direction: str):
        if direction == "0":
            return self.left
        if direction == "1":
            return self.right
        


def get_huffman_codes(symbol_length: List[Tuple[int, int]]):
    """print huffman table from lengths"""

    # Count the number of codes for each code length.
    bl_count = defaultdict(lambda: 0)
    next_code = dict()
    max_length = 0
    for _, length in symbol_length:
        bl_count[length] += 1
        max_length = max(length, max_length)

    # Find the numerical value of the smallest code for each code length
    code = 0
    for bits in range(1, max_length+1):
        code = (code + bl_count[bits-1]) << 1;
        next_code[bits] = code;

    symbol_code = list()
    for symbol, length in symbol_length:
        if length:
            symbol_code.append((symbol, next_code[length], length))
            next_code[length] += 1

    return symbol_code



if __name__ == "__main__":
    symbol_lengths = []
    symbol_lengths.extend([(x, 8) for x in range(0, 144)])
    symbol_lengths.extend([(x, 9) for x in range(144, 256)])
    symbol_lengths.extend([(x, 7) for x in range(256, 280)])
    symbol_lengths.extend([(x, 8) for x in range(280, 288)])

    huffman_codes = get_huffman_codes(symbol_lengths)

    root = BTreeNode(UndefinedNode(), UndefinedNode())
    #root.traverse_create((bin(huffman_codes[0][1])[2:]).zfill(huffman_codes[0][2]), Symbol(huffman_codes[0][0]))
    for symbol, code, length in huffman_codes:
        code_path = (bin(code)[2:]).zfill(length)
        symbol = Symbol(symbol)
        root.traverse_create(code_path, symbol)

    #print(f"symbol\t\tcode\t\tbin(code)")
    #for symbol, code, length in huffman_codes:
    #    print(f"{symbol}\t\t{code}\t\t{(bin(code)[2:]).zfill(length)}")

    # one red pixel deflate compressed with static huffman codes
    deflate_block = b'\x63\xF8\xCF\xC0\x00\x00'

    bitstream = reduce(lambda a,b: a+b, [(bin(x)[2:]).zfill(8)[::-1] for x in deflate_block])
    print(f"{bitstream=}")

    pos = 0

    bfinal = int(bitstream[pos:pos+1][::-1], 2)
    pos += 1
    bitstream_rest = bitstream[pos:]

    print(f"{bfinal=}")
    print(f"{bitstream_rest=}")

    btype = int(bitstream[pos:pos+2][::-1], 2)
    pos += 2
    bitstream_rest = bitstream[pos:]

    print(f"{btype=}")
    print(f"{bitstream_rest=}")

    parsed_symbols = []
    current_node = root
    while pos < len(bitstream):
        current_node = current_node.traverse_once(bitstream[pos])
        if isinstance(current_node, Symbol):
            parsed_symbols.append(current_node.value)
            current_node = root
        pos += 1

    print(parsed_symbols)

