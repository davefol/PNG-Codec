import sys
import subprocess

from upng import view

print("python3 disp_datagz.py in out height width channels")

with open(sys.argv[1], "rb") as fp:
    deflate_stream = fp.read()
    gzip_header = b"\x1f\x8b\x08\x00\x00\x00\x00\x00\x02\x03"
    compliant_gzip = gzip_header + deflate_stream
    compliant_gzip += (0).to_bytes(8, "big")

with open(sys.argv[2], "wb") as fp:
    fp.write(compliant_gzip)

gzip_out = subprocess.run([
    "gzip",  "-dc", sys.argv[2]
], capture_output=True).stdout

view(gzip_out, int(sys.argv[3]), int(sys.argv[4]), int(sys.argv[5]))
