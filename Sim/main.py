import pygame
import sys

#hello anhad
# create a 64x64 pygame window
# record the mouse cursor position and display it onscrewen

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