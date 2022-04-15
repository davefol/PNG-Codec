import numpy as np
import matplotlib.pyplot as plt

def as_uint32(arr: np.ndarray) -> int:
    out = 0
    out |= (arr[0] << 3 * 8) 
    out |= (arr[1] << 2 * 8) 
    out |= (arr[2] << 1 * 8) 
    out |= (arr[3] << 0 * 8) 
    return out

def as_uint24(arr: np.ndarray) -> int:
    out = 0
    out |= (arr[2] << 2 * 8) 
    out |= (arr[1] << 1 * 8) 
    out |= (arr[0] << 0 * 8) 
    return out

def view(data: bytes, height: int, width: int, channels: int):
    img = np.zeros((height, width, channels), np.uint8)
    iter_bytes = iter(data)
    for h in range(height):
        filter_type = next(iter_bytes)
        for w in range(width):
            img[h, w] =  [next(iter_bytes) for _ in range(channels)]
            if (filter_type == 1 and w > 0):
                img[h, w] = img[h, w] + img[h, w-1]
            elif (filter_type == 2 and h > 0):
                img[h, w] = img[h, w] + img[h-1, w]
            # Average(x) + floor((Raw(x-bpp)+Prior(x))/2)
            elif (filter_type == 3):
                up_pix = np.array([0] * channels) if h == 0 else img[h-1, w]
                left_pix = np.array([0] * channels) if w == 0 else img[h, w-1]
                img[h, w] += np.floor((up_pix.astype(np.uint16) + left_pix.astype(np.uint16)) / 2).astype(np.uint8)
            elif (filter_type == 4):
                up_pix = np.array([0] * channels) if h == 0 else img[h-1, w]
                left_pix = np.array([0] * channels) if w == 0 else img[h, w-1]
                up_left_pix = np.array([0] * channels) if (w == 0 or h == 0) else img[h-1, w-1]

                up_pix = up_pix.astype(np.uint16)
                left_pix = left_pix.astype(np.uint16)
                up_left_pix = up_left_pix.astype(np.uint16)

                for channel in range(channels):
                    base_value = left_pix[channel].item() + up_pix[channel].item() - up_left_pix[channel].item()
                    predictor = min([
                        (abs(left_pix[channel].item() - base_value),0, left_pix[channel]),
                        (abs(up_pix[channel].item() - base_value),1, up_pix[channel]),
                        (abs(up_left_pix[channel].item() - base_value),2, up_left_pix[channel]),
                        ], key= lambda x:x[0])[2]
                    img[h, w][channel] += predictor


                # function PaethPredictor (a, b, c)
                # begin
                #      ; a = left, b = above, c = upper left
                #      p := a + b - c        ; initial estimate
                #      pa := abs(p - a)      ; distances to a, b, c
                #      pb := abs(p - b)
                #      pc := abs(p - c)
                #      ; return nearest of a,b,c,
                #      ; breaking ties in order a,b,c.
                #      if pa <= pb AND pa <= pc then return a
                #      else if pb <= pc then return b
                #      else return c
                # end
                

    if channels > 3:
        img = img[:, :, :3]
    return img

def view2(data1, data2, height, width, channels):
    a = view(data1, height, width, channels)
    b = view(data2, height, width, channels)
    fig, ax = plt.subplots(ncols=2)
    ax[0].imshow(a)
    ax[1].imshow(b)
    plt.show()

                

