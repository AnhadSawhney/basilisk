# import skin/skin1.png and print out the contents as an array

import numpy as np
from PIL import Image

# Load image
im = Image.open('skin/skin1.png')
#im = Image.open('eyeball/eyeball_full_test.png')
rgba_im = im.convert("RGBA")
pix = rgba_im.load()

# Get image data
#image_array = list(rgba_im.getdata())
size = rgba_im.height
arr = [[rgba_im.getpixel((i,j)) for j in range(size)] for i in range(size)]


def print_image():
    out = '' 
    for i in range(0,size):
        for j in range(0,size):
            #if(arr[i][j][3] == 0):
            #    print('{0,255,0}', end=',')
            #else:
            print('{'+str(arr[j][i][0])+','+str(arr[j][i][1])+','+str(arr[j][i][2])+'}', end=',')
            out += '{'+str(arr[j][i][0])+','+str(arr[j][i][1])+','+str(arr[j][i][2])+'},'
        print()
        out += "\n"

    # save out to a file called array.txt
    f = open("array.txt", "w")
    f.write(out)
    f.close()

def print_mask():
    for i in range(0,size):
        print('0b', end='')
        for j in range(0,size):
            if(arr[j][i][3] == 0):
                print('1', end='')
            else:
                print('0', end='')
        print(',')

print_mask()
#print_image()