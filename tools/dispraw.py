import sys

import matplotlib.pyplot as plt

from upng import view

with open(sys.argv[1], "rb") as fp:
    uncompressed_stream = fp.read()

plt.imshow(view(uncompressed_stream, int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4])))
plt.show()
