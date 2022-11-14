from PIL import Image
file1 = 'skin/blinking skin only1.png'
file2 = 'skin/blinking skin only13.png'

# read file1 and file2 and print whether they are the same or not

def compare(a,b):
    im1 = Image.open(a)
    im2 = Image.open(b)
    if im1 == im2:
        print('same')
    else:
        print('different')

def main():
    for i in range(13,20):
        file1 = 'skin/blinking skin only'+str(i)+'.png'
        file2 = 'skin/blinking skin only'+str(28+13-i)+'.png'
        compare(file1,file2)

if __name__ == '__main__':
    main()