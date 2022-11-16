import pygame
import sys

#hello anhad
# TODO: 
# 1. Draw the eyeball (select the image closest to the mouse cursor position)
# 2. Draw the skin
# 3. Animate the blinking (choose a random time to weight between blinks and display the frames in sequence)
# Stretch:
# 4. Isolate the iris from the eyeball images
# 5. Draw the iris at the exact correct position
# 6. Blend between the eyeball backgrounds to get a smooth transition between positions

def main():
    pygame.init()
    screen = pygame.display.set_mode((64,64))
    pygame.display.set_caption('The All Seeing Eye')
    clock = pygame.time.Clock()
    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
        screen.fill((0,0,0))
        mouse_pos = pygame.mouse.get_pos()
        print(mouse_pos)
        pygame.display.update()
        clock.tick(60)

# load all the images in the eyeball folder and save them in a list
# load all the images in the skin folder and save them in a list
def load_eyeball_images():
    eyeball = []
    for i in range(1,14):
        eyeball.append(pygame.image.load('eyeball/eyeball'+str(i)+'.png'))
    return eyeball

def load_skin_images():
    skin = []
    for i in range(1,14):
        skin.append(pygame.image.load('skin/skin'+str(i)+'.png'))
    return skin

if __name__ == '__main__':
    main()