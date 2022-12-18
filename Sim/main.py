import pygame
import sys

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

screen = pygame.display.set_mode((64,64))

def main():
    pygame.init()
    pygame.display.set_caption('The All Seeing Eye')
    clock = pygame.time.Clock()
    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            if event.type == pygame.MOUSEBUTTONUP:
                blink()
        #screen.fill((0,0,0))
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
    skin = [pygame.image.load('skin/blinking skin only1.png')]
    for i in range(13,21):
        skin.append(pygame.image.load('skin/blinking skin only'+str(i)+'.png'))
    return skin

def blink():
    #close eye
    skins = load_skin_images()
    print(len(skins))
    for i in range(0,9):
        #print(i)
        screen.fill((0,0,0))
        screen.blit(skins[i], (0,0))
        pygame.display.update()
        pygame.time.delay(150)
    #open eye
    for i in range(7, -1, -1):
        #print(i)
        screen.fill((0,0,0))
        screen.blit(skins[i], (0,0))
        pygame.display.update()
        pygame.time.delay(150)
    screen.fill((0,0,0))

def look_at_mouse():
    mouse_pos = pygame.mouse.get_pos()
    #TODO

if __name__ == '__main__':
    main()