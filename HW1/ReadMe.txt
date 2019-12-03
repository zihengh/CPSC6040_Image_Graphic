ZiHeng He
zihengh@g.clemson.edu

First type make command after entering the project folder
type ./imgview img1.png img2.jpg to start the program (img1.png and img2.jpg are the image file you'd like to display at first)

Key functions are as follows:

1. press left button and the right button to view the previous or next image you typed in command line

2. press 1,2,3 to view the red, green and blue channel of the image on display, press o to convert it back to the original image

3. press i to view the image of whose colors are inverted.

4. you can always resize the window with your mouse and notice that the image is uniformly shaped in the center of the window.

5. you can always press r to retype the image filenames you want to display. The command line should be formed as 
imgview xxx.jpg xxx.png xxx.ppm
The filenames should be split by " ", and the command will not be executed if not starts with "imgview". Please notice that all the image files must be put in the project folder, the same directory of the execution file.

6. you can always press w to write the image you're viewing in the window. Please notice that if you are viewing the original image, the image will be stored with its original size, no matter how you resize the window with your mouse.
However, the the window being displayed will be stored as image if you are viewing the red, green or blue channel of the image.
