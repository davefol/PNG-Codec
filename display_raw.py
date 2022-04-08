from pathlib import Path
import numpy as np
import matplotlib.pyplot as plt

img_bytes = Path("examples/smile.raw").read_bytes()
height = 440
width = 440
channels = 4
print(len(img_bytes), width * height * 4)

img = np.zeros((height, width, channels), np.uint8)

iter_bytes = iter(img_bytes)
try:
    for row in range(height):
        filter_byte = next(iter_bytes)
        print(hex(filter_byte))
        for col in range(width):
            img[row][col][0] = next(iter_bytes)
            img[row][col][1] = next(iter_bytes)
            img[row][col][2] = next(iter_bytes)
            img[row][col][3] = next(iter_bytes)
except:
    print("not enough bytes")

plt.imshow(img)
plt.show()
        


