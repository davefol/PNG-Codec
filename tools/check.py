"""
Check that huffman codes, literals, repeats, and zeros
matches Mark Adlers infgen output

Also check that the uncompressed output matches
gzip output
"""
from pprint import pprint
import sys
import subprocess
from crc32 import crc32
from upng import view2

if __name__ == "__main__":
    input_png = sys.argv[1]
    # Run our **scuffed** PNG decoder on a PNG
    viewer_output = subprocess.run(
        ["../build/viewer", input_png, "--infgen"], capture_output=True
    ).stdout

    observed_codes = []
    for line in viewer_output.split(b"\n"):
        if line.startswith(b"code"):
            tokens = line.split(b" ")
            symbol = int(tokens[1])
            length = int(tokens[2])
            observed_codes.append((symbol, length))

    observed_huffman = [
        line for line in viewer_output.split(b"\n")
        if line.startswith(b"lens") or line.startswith(b"repeat") or line.startswith(b"zero")
    ]

    # Run infgen on data.gz
    infgen_output = subprocess.run(
        ["./infgen", "-dd", "data.gz"], capture_output=True
    ).stdout

    expected_codes = []
    for line in infgen_output.split(b"\n"):
        if line.startswith(b"code"):
            tokens = line.split(b" ")
            symbol = int(tokens[1])
            length = int(tokens[2].split(b"!")[0])
            expected_codes.append((symbol, length))

    expected_huffman = [
        line for line in infgen_output.split(b"\n")
        if line.startswith(b"lens") or line.startswith(b"repeat") or line.startswith(b"zero")
    ]

    # We compare (intelligent diffing)
    if observed_codes == expected_codes:
        print("✅ code lengths")
    else:
        print("❌ code lengths")
        print(f"\tobserved_codes: {len(observed_codes)}, expected_codes{len(expected_codes)}")
        breakpoint()
        quit()

    observed_lz = []
    for line in viewer_output.split(b"\n"):
        if line.startswith(b"literal"):
            observed_lz.append(("literal", int(line.split(b"literal")[1])))
        elif line.startswith(b"match"):
            _, l, d = line.split(b" ")
            observed_lz.append(("match", (int(l), int(d))))

    expected_lz = []
    for line in infgen_output.split(b"\n"):
        if line.startswith(b"literal"):
            expected_lz.append(("literal", line.split(b"literal")[1].split(b"\t\t")[0]))
        elif line.startswith(b"match"):
            # b'match 258 1\t\t! 0 010'
            expected_lz.append(("match", (line.split(b" ")[1], line.split(b" ")[2].split(b"\t\t")[0])))

    for i, _ in enumerate(expected_lz):
        t, v = expected_lz[i]
        if t == "literal" and b"'" in v:
            expected_lz[i] = (expected_lz[i][0], ord(v[v.index(b"'")+1:]))
        elif t == "literal" and b"'" not in v:
            expected_lz[i] = (expected_lz[i][0], int(expected_lz[i][1]))
        else:
            l, d  = expected_lz[i][1]
            expected_lz[i] = (expected_lz[i][0], (int(l), int(d)))

    mismatch = []
    for i, (x, y) in enumerate(zip(expected_lz, observed_lz)):
        if x != y:
            mismatch.append((i, (x, y)))
            
    # We compare (intelligent diffing)
    if len(mismatch) == 0:
        print("✅ LZ77")
    else:
        print("❌ LZ77")
        for x in mismatch:
            print(x)
        quit()

    with open("data.gz", "rb") as fp:
        deflate_stream = fp.read()
        gzip_header = b"\x1f\x8b\x08\x00\x00\x00\x00\x00\x02\x03"
        compliant_gzip = gzip_header + deflate_stream

    with open("data.raw", "rb") as fp:
        uncompressed_stream = fp.read()
        compliant_gzip += crc32(uncompressed_stream).to_bytes(4, "big")
        compliant_gzip += (len(uncompressed_stream) % (1 << 32)).to_bytes(4, "big")

    with open("compliant.gz", "wb") as fp:
        fp.write(compliant_gzip)

    gzip_out = subprocess.run([
        "gzip",  "-dc", "./compliant.gz"
    ], capture_output=True).stdout


    h_line, w_line, b_line, c_line = viewer_output.split(b"\n")[6:10]
    img_height = int(h_line.split(b":")[-1])
    img_width = int(w_line.split(b":")[-1])
    img_bit_depth = int(b_line.split(b":")[-1])
    img_color_type = int(c_line.split(b":")[-1])
    #view(gzip_out, img_height, img_width, 4 if img_color_type == 6 else 3)
    #view2(gzip_out, uncompressed_stream, img_height, img_width, 4 if img_color_type == 6 else 3)

    if (gzip_out == uncompressed_stream):
        print("✅ decompress")
    else:
        print("❌ decompress")

    for i, (expected_byte, observed_byte) in enumerate(zip(gzip_out, uncompressed_stream)):
        if expected_byte != observed_byte:
            print(f"[{i}] expected={hex(expected_byte)}, observed={hex(observed_byte)}")
            quit()


