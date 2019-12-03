HW5:Warper
zihengh

The project contains five files: warper.cpp, matrix.cpp, matric.h, NonlineaerMap.cpp and NonlinearMap.h

To start the program, type ./warper <InputIamge> <OutputImage>(optional) <mode>
for instance: ./warper Death.jpg waperedImg.png 

You can choose the mode at the beginning as follows:
default mode: projective mode
mode switch:
  -b          bilinear switch - do the bilinear warp instead of a perspective warp
  -i          interactive switch

When in a projective mode or in a bilinear mode(-b), the programs starts with waiting for warper thresholds. The program will accept all the thresholds until you entered "d"

matrix commands:
  r theta     counter clockwise rotation about image origin, theta in degrees
  s sx sy     scale (watch out for scale by 0!)
  t dx dy     translate
  f xf yf     flip - if xf = 1 flip horizontal, yf = 1 flip vertical
  h hx hy     shear
  p px py     perspective
  n cx cy s   twirl
  m tx ty ax ay ripple
  d           done

Be noticed, twirl and ripple are nonlinear warper, which means twirl and ripple thresholds cannot be used together with others.

While in a interactive mode, you need to click the window for four times to set the corners of the output image. The window size is fixed as 1024*768 as well as the output image size is. The border of output image will be marked with red lines but these red lines will not be saved into image file.


