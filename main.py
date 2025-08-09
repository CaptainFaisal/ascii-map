from itertools import groupby
from PIL import Image


img_path = 'map.png'
img = Image.open(img_path)
width, height = img.size
img_data = list(map(lambda p: sum(p), img.getdata()))
img_matrix = [img_data[i:i+width] for i in range(0, len(img_data), width)]

# cell size
W, H = 10, 20
offsetX = W // 2
offsetY = H // 2

data = []

for j in range(height // H):
    for i in range(width // W):
        x, y = (i * W, j * H)
        # move the points in the middle
        if img_matrix[y + offsetY][x + offsetX] != 0:
            print('*', end='')
            data.append(1)
        else:
            print(' ', end='')
            data.append(0)
    print()

group = [len(list(g)) for k, g in groupby(data)]
# remapping the chars
encodedString = ''.join([chr(126 - val) for val in group])

# This is the magic string
print(repr(encodedString))

