# import skin/skin1.png and print out the contents as an array

import numpy as np
from PIL import Image

# Load image
im = Image.open('skin/skin1.png')
rgba_im = im.convert("RGBA")
pix = rgba_im.load()

# Get image data
#image_array = list(rgba_im.getdata())
arr = [[rgba_im.getpixel((i,j)) for j in range(64)] for i in range(64)]

def print_image():
    for i in range(0,64):
        for j in range(0,64):
            #if(arr[i][j][3] == 0):
            #    print('{0,255,0}', end=',')
            #else:
            print('{'+str(arr[i][j][0])+','+str(arr[i][j][1])+','+str(arr[i][j][2])+'}', end=', ')
        print()

def print_mask():
    for i in range(0,64):
        print('0b', end='')
        for j in range(0,64):
            if(arr[i][j][3] == 0):
                print('1', end='')
            else:
                print('0', end='')
        print(',')


print_mask()