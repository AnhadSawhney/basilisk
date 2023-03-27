import pygame
import sys
from PIL import Image

# create a 64x64 pygame window
# record the mouse cursor position and display it onscrewen
# TODO: 
# 1. Draw the eyeball (select the image closest to the mouse cursor position)
# 2. Draw the skin
# 3. Animate the blinking (choose a random time to weight between blinks and display the frames in sequence)
# Stretch:
# 4. Isolate the iris from the eyeball images
# 5. Draw the iris at the exact correct position
# 6. Blend between the eyeball backgrounds to get a smooth transition between positions

#screen = pygame.display.set_mode((256,256))
# create a screen that can be resized, starts at 256x256
screen = pygame.display.set_mode((256,256), pygame.RESIZABLE)
pixel_scale = screen.get_width() / 64
array_2d = []
blink = 0

#takes a loaded png image and turns it into a 2d pixel array
def array_from_image(path):
    #turn image into pixel array
    #64x64 image -> one array with 64 arrays with 64 elements each (r,c)
    #(R,G,B,A)
    im = Image.open(path)
    rgba_im = im.convert("RGBA")
    pix = rgba_im.load()
    image_array = list(rgba_im.getdata())
    array_2d = [[rgba_im.getpixel((i,j)) for j in range(64)] for i in range(64)]
    return array_2d

skins = []
for i in range(1,10):
    skins.append(array_from_image('skin/skin'+str(i)+'.png'))

def composite_array(over, under, alpha):
    for i in range(0,64):
        for j in range(0,64):
            if over[i][j][3] > 0:
                under[i][j] = over[i][j]
            #alpha = over[i][j][3]
            # TODO: element wise multiplication
            #under[i][j] = under[i][j] * (255-alpha) + over[i][j] * alpha

#pupil data
# offset, width
pupil_data = [
    [-2, 2],
    [-3, 4],
    [-3, 5],
    [-4, 7],
    [-4, 7],
    [-5, 8],
    [-5, 9],
    [-5, 9],
    [-5, 10],
    [-5, 10],
    [-5, 10],
    [-5, 10],
    [-5, 10],
    [-5, 11],
    [-5, 11],
    [-5, 11],
    [-6, 12],
    [-6, 12],
    [-6, 12],
    [-6, 12],
    [-6, 12],
    [-6, 12],
    [-6, 12],
    [-6, 13],
    [-6, 13],
    [-5, 12],
    [-5, 12],
    [-5, 12],
    [-5, 12],
    [-5, 12],
    [-5, 12],
    [-5, 12],
    [-5, 11],
    [-5, 11],
    [-5, 11],
    [-4, 10],
    [-4, 10],
    [-4, 10],
    [-4, 10],
    [-4, 10],
    [-3, 9],
    [-3, 9],
    [-2, 8],
    [-2, 8],
    [-2, 7],
    [-2, 7],
    [-1, 5],
    [0, 4],
    [1,2]
]

pupil_height = len(pupil_data)

def render_pupil(pos):
    global array_2d
    #print(pos)
    #print(pupil_data)
    #print(pupil_height)
    for i in range(pupil_height):
        y = int(pos[1] - pupil_height/2 + i)
        if y < 0 or y > 63:
            continue
        offset = pupil_data[i][0]

        # move the offset if the pupil is close to the edge
        x = int(pos[0])
        alpha = (x - 32)/1.5
        offset += alpha*(1-(1-y/32)**2)

        width = pupil_data[i][1]
        for j in range(width):
            x = int(pos[0] + j + offset)
            if x < 0 or x > 63:
                continue
            array_2d[x][y] = [0,0,0,255]

def update_array():
    global blink
    global array_2d
    array_2d = [[[255,255,255,255] for j in range(64)] for i in range(64)]
    pos = pygame.mouse.get_pos()
    pos = (int(pos[0]/pixel_scale), int(pos[1]/pixel_scale))
    render_pupil(pos)

    if blink == -1:
        blink = 1
    if blink >= len(skins):
        composite_array(skins[2*len(skins)-blink-1], array_2d, 255)
        #print(2*len(skins)-blink-1)
    else:
        composite_array(skins[blink], array_2d, 255)
        #print(blink)
    if blink > 0:
        blink += 1
        if blink >= len(skins)*2:
            blink = 0
    

def draw_from_array():
    global array_2d
    global pixel_scale
    # draw the 64x64 array on the screen, scaling it to the size of the screen
    for i in range(0,64):
        for j in range(0,64):
            pygame.draw.rect(screen, array_2d[i][j], (i*pixel_scale,j*pixel_scale,pixel_scale,pixel_scale))

def main():
    pygame.init()
    pygame.display.set_caption('The All Seeing Eye')
    clock = pygame.time.Clock()
    global screen, pixel_scale, blink
    #iris_only = pygame.image.load('eyeball/iris_only.png')
    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            if event.type == pygame.MOUSEBUTTONUP:
                if blink == 0:
                    blink = -1 # trigger blink
            # handle screen resize
            if event.type == pygame.VIDEORESIZE:
                size = min(event.w, event.h)
                size = size - (size % 64)
                screen = pygame.display.set_mode((size, size), pygame.RESIZABLE)
                pixel_scale = size / 64
        #screen.fill((0,0,0))
        #move_pupil()
        update_array()
        draw_from_array()
        pygame.display.update()
        clock.tick(33)

if __name__ == '__main__':
    main()