CRC_TABLE = [None] * 256

for n in range(256):
    c = n
    for k in range(8):
        if c & 1:
            c = 0xedb88320 ^ (c >> 1)
        else:
            c = c >> 1
    CRC_TABLE[n] = c

def crc32(data: bytes) -> int:
    out = 0
    out ^= 0xffffffff
    for x in data:
        out = CRC_TABLE[(out ^ x) & 0xFF] ^ (out >> 8)
    out ^= 0xffffffff
    return out

assert 0x00000000 == crc32(b"")
assert 0x414FA339 == crc32(b"The quick brown fox jumps over the lazy dog")
assert 0x9BD366AE == crc32(b"various CRC algorithms input data")
assert 0x0C877F61 == crc32(b"Test vector from febooti.com")
