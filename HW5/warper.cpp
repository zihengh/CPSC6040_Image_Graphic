# include <OpenImageIO/imageio.h>
# include <stdlib.h>
# include <cstdlib>
# include <iostream>
# include <fstream>
# include <string>
# include <algorithm>
# include <math.h>
# include <cmath>
# include <iomanip>
# include "matrix.h"
# include "NonlinearMap.h"

# ifdef __APPLE__
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#   include <GLUT/glut.h>
# else
#   include <GL/glut.h>
# endif

using namespace std;
OIIO_NAMESPACE_USING

# define max(x, y) (x > y ? x : y)
# define min(x, y) (x < y ? x : y)

# define INTERACTIVE_SCREEN_WIDTH 1024
# define INTERACTIVE_SCREEN_HEIGH 768


static Matrix3D transMatrix;  // transform matrix for the entire transform
static Matrix3D translation;  // extra translation transform matrix
static unsigned char* puchInputPixmap = NULL;  // input image pixels pixmap
static unsigned char* puchOutputPixmap = NULL; // output image pixels pixmap
static string strInputImage;  // input image file name
static string strOutputImage; // output image file name
static int nWidthIn, nHeightIn;  // input image size: width, height
static int nWidthOut, nHeightOut;  // output image size: width, height
static double x_min, y_min;
static int mode;  // program mode - 0: projective warp (basic requirement), 1: bilinear warp, 2: interactive mode
static Vector2D mouseClickCorners[4];
static int mouse_index = 0;
static NonlinearMap AdvMap;
static int inversemode = 0;


enum
{
  INVERSE_LINEAR=0,
  INVERSE_TWIRL,
  INVERSE_RIPPLE,
};

enum
{
  MODE_PROJECTIVE=0,
  MODE_BILINEAR,
  MODE_INTERACTIVE,
};

void CreateOutputPixel()
{
  if(puchOutputPixmap != NULL)
  {
    delete []puchOutputPixmap;
  }
  puchOutputPixmap = new unsigned char[nWidthOut*nHeightOut*4];
  memset(puchOutputPixmap, 0, nWidthOut*nHeightOut*4*sizeof(unsigned char));
}

void ReleasePixels()
{
  if(puchOutputPixmap != NULL)
  {
    delete []puchOutputPixmap;
    puchOutputPixmap=NULL;
  }
  if(puchInputPixmap != NULL)
  {
    delete []puchInputPixmap;
    puchInputPixmap=NULL;
  }
}

void helpPrinter()
{
  // print help message
  cout << "Help: " << endl;
  cout << "[Usage] warper input_image_name [output_image_name] [mode]" << endl;
  cout << "--------------------------------------------------------------" << endl;
  cout << "default mode: projective warp" << endl;
  cout << "mode switch: " << endl;
  cout << "\t-b             bilinear switch - do the bilinear warp instead of a perspective warp\n"
       << "\t-i             interactive switch" << endl;
  cout << "matrix commands: " << endl;
  cout << "\tr theta        counter clockwise rotation about image origin, theta in degrees\n"
       << "\ts sx sy        scale (watch out for scale by 0!)\n"
       << "\tt dx dy        translate\n"
       << "\tf xf yf        flip - if xf = 1 flip horizontal, yf = 1 flip vertical\n"
       << "\th hx hy        shear\n"
       << "\tp px py        perspective\n"
       << "\tn cx cy s      twirl\n"
       << "\tm tx ty ax ay  ripple\n"
       << "\td              done\n" << endl;
}

char **getIter
(
  char** begin, char** end, const std::string& option) {return find(begin, end, option);
}

void getCmdOptions(int argc, char **argv, string &strInputImage, string &strOutputImage)
{
  // print help message and exit the program
  if (argc < 2)
  {
    helpPrinter();
    exit(0);
  }
  mode = MODE_PROJECTIVE;
  char **iter = getIter(argv, argv + argc, "-b");
  if (iter != argv + argc)  {mode = MODE_BILINEAR;  cout << "program mode: bilinear warp" << endl;}
  else
  {
    iter = getIter(argv, argv + argc, "-i");
    if (iter != argv + argc)  {mode = MODE_INTERACTIVE;  cout << "program mode: interactive" << endl;}
  }
  strInputImage = argv[1];
  if (mode == MODE_PROJECTIVE)
  {
    cout << "program mode: projective warp" << endl;
    if (argc == 3)  {strOutputImage = argv[2];}
  }
  else
  {
    if (argc == 4)  {strOutputImage = argv[2];}
  }
}


/*
matrix command parser
  calculate forward transform matrix
*/
void generateMatrix()
{
  cout << "Please enter matrix commands: " << endl;
  Matrix3D xform;
  char tag;
  cin >> tag;
  while (tag != 'd')
  {
    if (tag != 'r' && tag != 's' && tag != 't' && tag != 'f' && 
        tag != 'h' && tag != 'p' && tag != 'n' && tag != 'm' && tag != 'd') 
    {helpPrinter(); exit(0);}
    // generate transform matrix
    switch (tag)
    {
      // rotation
      case 'r':
        double theta;
        cin >> theta;
        xform[0][0] = xform[1][1] = cos(theta * M_PI / 180);
        xform[0][1] = -sin(theta * M_PI / 180);
        xform[1][0] = sin(theta * M_PI / 180);
        break;
      // scale
      case 's':
        double sx, sy;
        cin >> sx >> sy;
        xform[0][0] = sx;
        xform[1][1] = sy;
        break;
      // translate
      case 't':
        double dx, dy;
        cin >> dx >> dy;
        xform[0][2] = dx;
        xform[1][2] = dy;
        break;
      // flip: if xf = 1 flip horizontal, yf = 1 flip vertical
      case 'f':
        double xf, yf;
        cin >> xf >> yf;
        if (xf == 1)  {xform[0][0] = -1;}
        if (yf == 1)  {xform[1][1] = -1;}
        break;
      // shear
      case 'h':
        double hx, hy;
        cin >> hx >> hy;
        xform[0][1] = hx;
        xform[1][0] = hy;
        break;
      // perspective
      case 'p':
        double px, py;
        cin >> px >> py;
        xform[2][0] = px;
        xform[2][1] = py;
        break;
      case 'n':
        double cx, cy, s;
        cin >> cx >> cy >> s;
        inversemode = INVERSE_TWIRL;
        AdvMap.SetTwirl(cx, cy, s);
        break;
      case 'm':
        double tx, ty, ax, ay;
        cin >> tx >> ty >> ax >> ay;
        inversemode = INVERSE_RIPPLE;
        AdvMap.SetRipple(tx, ty, ax, ay);
        break;
      default:
        return;
    }
    cin >> tag;
  }
  transMatrix = xform * transMatrix;
}


/*
four corners forward warp to make space for output image pixmap
*/
void boundingbox(Vector2D xycorners[])
{
  Vector2D u0, u1, u2, u3;
  u0.x = 0;
  u0.y = 0;
  u1.x = 0;
  u1.y = nHeightIn;
  u2.x = nWidthIn;
  u2.y = nHeightIn;
  u3.x = nWidthIn;
  u3.y = 0;

  //Vector2D xy0, xy1, xy2, xy3;
  xycorners[0] = transMatrix * u0;
  xycorners[1] = transMatrix * u1;
  xycorners[2] = transMatrix * u2;
  xycorners[3] = transMatrix * u3;

  double x0, y0, x1, y1, x2, y2, x3, y3, x_max, y_max;
  x0 = xycorners[0].x;
  y0 = xycorners[0].y;
  x1 = xycorners[1].x;
  y1 = xycorners[1].y;
  x2 = xycorners[2].x;
  y2 = xycorners[2].y;
  x3 = xycorners[3].x;
  y3 = xycorners[3].y;
  
  // calculate output image size
  x_max = max(max(x0, x1), max(x2, x3));
  x_min = min(min(x0, x1), min(x2, x3));
  y_max = max(max(y0, y1), max(y2, y3));
  y_min = min(min(y0, y1), min(y2, y3));
  if(mode != MODE_INTERACTIVE)
  {
    nWidthOut = ceil(x_max - x_min);
    nHeightOut = ceil(y_max - y_min);
  }
  
  if (mode != MODE_INTERACTIVE)  {cout << "output image size: " << nWidthOut << "x" << nHeightOut << endl;}
  
  // calculate extra translation
  // extra translation transform: set x_min and y_min to 0
  translation[0][2] = 0 - x_min;
  translation[1][2] = 0 - y_min;
}


/*
projective warp inverse map
*/
void inversemap()
{
  CreateOutputPixel();

  Matrix3D invMatrix;
  transMatrix = translation * transMatrix;
  cout << "transform matrix: " << endl;
  transMatrix.print();
  invMatrix = transMatrix.inverse();
  cout << "inverse matrix: " << endl;
  invMatrix.print();

  for (int row_out = 0; row_out < nHeightOut; row_out++)  // output image row
  {
    for (int col_out = 0; col_out < nWidthOut; col_out++)  // output image col
    {
      // output coordinate
      Vector2D xy;
      xy.x = col_out + 0.5;
      xy.y = row_out + 0.5;

      // inverse mapping
      double u, v;
      u = (invMatrix * xy).x;
      v = (invMatrix * xy).y;
      int row_in, col_in;
      row_in = floor(v);
      col_in = floor(u);
      
      if (row_in < nHeightIn && row_in >= 0 && col_in < nWidthIn && col_in >= 0)
      {
        for (int k = 0; k < 4; k++)
        {puchOutputPixmap[(row_out * nWidthOut + col_out) * 4 + k] = puchInputPixmap[(row_in * nWidthIn + col_in) * 4 + k];}
      }
    }
  }
  cout << "Projective inverse complete." << endl;
  cout << "Press Q or q to quit." << endl;
}


/*
bilinear warp inverse map
*/
void bilinear(Vector2D xycorners[])
{
  CreateOutputPixel();

  // translate the corners
  for (int i = 0; i < 4; i++) {xycorners[i] = translation * xycorners[i];}

  BilinearCoeffs coeff;
  setbilinear(nWidthIn, nHeightIn, xycorners, coeff);
  for (int row_out = 0; row_out < nHeightOut; row_out++)
  {
    for (int col_out = 0; col_out < nWidthOut; col_out++)
    {
      Vector2D xy, uv;

      xy.x = col_out + 0.5;
      xy.y = row_out + 0.5;
      invbilinear(coeff, xy, uv);

      int row_in, col_in;
      row_in = floor(uv.y);
      col_in = floor(uv.x);

      if (row_in < nHeightIn && row_in >= 0 && col_in < nWidthIn && col_in >= 0)
      {
        for (int k = 0; k < 4; k++)
        {puchOutputPixmap[(row_out * nWidthOut + col_out) * 4 + k] = puchInputPixmap[(row_in * nWidthIn + col_in) * 4 + k];}
      }
    }
  }
  cout << "Bilinear inverse complete." << endl;
  cout << "Press Q or q to quit." << endl;
}




void interactive()
{
  // interMatrix
  // | a b c ||u|   |x|
  // | d e f ||v| = |y|
  // | g h 1 ||1|   |w|
  double x0, x1, x2, x3, y0, y1, y2, y3;
  x0 = mouseClickCorners[0].x;
  y0 = mouseClickCorners[0].y;
  x1 = mouseClickCorners[1].x;
  y1 = mouseClickCorners[1].y;
  x2 = mouseClickCorners[2].x;
  y2 = mouseClickCorners[2].y;
  x3 = mouseClickCorners[3].x;
  y3 = mouseClickCorners[3].y;

  double a, b, c, d, e, f;
  a = y1 - y2;
  b = y3 - y2;
  c = y0 - y2;
  d = x1 - x2;
  e = x3 - x2;
  f = x0 - x2;
  double w1, w3;
  w1 = (f - a * e / b) / (d - a * e / b);
  w3 = (f - c * d / a) / (e - b * d / a);

  transMatrix[0][0] = (x3 * w3 - x0) / nWidthIn;
  transMatrix[0][1] = (x1 * w1 - x0) / nHeightIn;
  transMatrix[0][2] = x0;
  transMatrix[1][0] = (y3 * w3 - y0) / nWidthIn;
  transMatrix[1][1] = (y1 * w1 - y0) / nHeightIn;
  transMatrix[1][2] = y0;
  transMatrix[2][0] = (w3 - 1) / nWidthIn;
  transMatrix[2][1] = (w1 - 1) / nHeightIn;

  // refresh the output image size and calculate extra translation

  boundingbox(mouseClickCorners);
  // add extra translation
  transMatrix = translation * transMatrix;
  cout << "transMatrix: " << endl;
  transMatrix.print();
  // inverse matrix
  Matrix3D invinterMatrix = transMatrix.inverse();
  cout << "inverseMatrix: " << endl;
  invinterMatrix.print();

  for (int row_out = 0; row_out < nHeightOut; row_out++)  // output image row
  {
    for (int col_out = 0; col_out < nWidthOut; col_out++)  // output image col
    {
      if(row_out < y_min || col_out < x_min)
      {
        continue;
      }
      // output coordinate
      Vector2D xy;
      xy.x = col_out + 0.5 - x_min;
      xy.y = row_out + 0.5 - y_min;

      // inverse mapping
      double u, v;
      u = (invinterMatrix * xy).x;
      v = (invinterMatrix * xy).y;
      int row_in, col_in;
      row_in = floor(v);
      col_in = floor(u);
      
      if (row_in  < nHeightIn && row_in  >= 0 && col_in  < nWidthIn && col_in  >= 0)
      {
        for (int k = 0; k < 4; k++)
        {
          puchOutputPixmap[(row_out * nWidthOut + col_out) * 4 + k] = puchInputPixmap[(row_in * nWidthIn + col_in) * 4 + k];
        }
      }
    }
  }
  cout << "Interactive complete." << endl;
  // resize the window
//  glutReshapeWindow(nWidthOut, nHeightOut);
  cout << "Press Q or q to quit." << endl;
}


/*
get the image pixmap
*/
void readimage(string infilename)
{
  // read the input image and store as a pixmap
  ImageInput *in = ImageInput::open(infilename);
  if (!in)
  {
    cerr << "Cannot get the input image for " << infilename << ", error = " << geterror() << endl;
    exit(0);
  }
  else
  {
    // get the image size and channels information, allocate space for the image
    const ImageSpec &spec = in -> spec();

    nWidthIn = spec.width;
    nHeightIn = spec.height;
    int channels = spec.nchannels;
    cout << "input image size: " << nWidthIn << "x" << nHeightIn << endl;
    cout << "channels: " << channels << endl;

    unsigned char tmppixmap[nWidthIn * nHeightIn * channels];
    int nScanlineSize = nWidthIn * channels * sizeof(unsigned char);
    if(!in -> read_image(TypeDesc::UINT8, tmppixmap+(nHeightIn-1)*nScanlineSize, AutoStride, -nScanlineSize))
    {
        cerr << "Could not read input image for " << infilename << ", error = " << geterror() << endl;
    }

    // convert input image to RGBA image
    puchInputPixmap = new unsigned char [nWidthIn * nHeightIn * 4];  // input image pixel map is 4 channels
    for (int row = 0; row < nHeightIn; row++)
    {
      for (int col = 0; col < nWidthIn; col++)
      {
        switch (channels)
        {
          case 1:
            puchInputPixmap[(row * nWidthIn + col) * 4] = tmppixmap[row * nWidthIn + col];
            puchInputPixmap[(row * nWidthIn + col) * 4 + 1] = puchInputPixmap[(row * nWidthIn + col) * 4];
            puchInputPixmap[(row * nWidthIn + col) * 4 + 2] = puchInputPixmap[(row * nWidthIn + col) * 4];
            puchInputPixmap[(row * nWidthIn + col) * 4 + 3] = 255;
            break;
          case 3:
            for (int k = 0; k < 3; k++)
            {puchInputPixmap[(row * nWidthIn + col) * 4 + k] = tmppixmap[(row * nWidthIn + col) * 3 + k];}
            puchInputPixmap[(row * nWidthIn + col) * 4 + 3] = 255;
            break;
          case 4:
            for (int k = 0; k < 4; k++)
            {puchInputPixmap[(row * nWidthIn + col) * 4 + k] = tmppixmap[(row * nWidthIn + col) * 4 + k];}
            break;
          default:
            return;
        }
      }
    }

    in -> close();  // close the file
    delete in;    // free ImageInput
  }
}


/*
write out the associated color image from image pixel map
*/
void writeimage(string outfilename)
{   
  // create the subclass instance of ImageOutput which can write the right kind of file format
  ImageOutput *out = ImageOutput::create(outfilename);
  if (!out)
  {
    cerr << "Could not create output image for " << outfilename << ", error = " << geterror() << endl;
    return ;
  }
  

    // .ppm file: 3 channels
  if (outfilename.substr(outfilename.find_last_of(".") + 1, outfilename.length() - 1) == "ppm")
  {
    unsigned char pixmap[nWidthOut * nHeightOut * 3];
    for (int row = 0; row < nHeightOut; row++)
    {
      for (int col = 0; col < nWidthOut; col++)
      {
        for (int k = 0; k < 3; k++)
        {pixmap[(row * nWidthOut + col) * 3 + k] = puchOutputPixmap[(row * nWidthOut + col) * 4 + k];}
      }
    }
    ImageSpec spec (nWidthOut, nHeightOut, 3, TypeDesc::UINT8);
    out -> open(outfilename, spec);
    int nScanlineSize = nWidthOut * 3 * sizeof(unsigned char);
    out -> write_image(TypeDesc::UINT8, pixmap+(nHeightOut-1)*nScanlineSize, AutoStride, -nScanlineSize);
    cout << "warped image stored " << outfilename << endl;
  
    out -> close();
    delete out;
    return ;
  }

  ImageSpec spec (nWidthOut, nHeightOut, 4, TypeDesc::UINT8);
  out -> open(outfilename, spec);
    // write the entire image
  int nScanlineSize = nWidthOut * 4 * sizeof(unsigned char);
  out -> write_image(TypeDesc::UINT8, puchOutputPixmap+(nHeightOut-1)*nScanlineSize, AutoStride, -nScanlineSize);
  cout << "warped image stored " << outfilename << endl;
  
  out -> close();
  delete out;
}


/*
display composed associated color image
*/
void display()
{
 
  // display the pixmap
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glRasterPos2i(0, 0);
  // glDrawPixels writes a block of pixels to the framebuffer
  glDrawPixels(nWidthOut, nHeightOut, GL_RGBA, GL_UNSIGNED_BYTE, puchOutputPixmap);
  if(mode == MODE_INTERACTIVE && mouse_index >= 3)
  {
    glLineWidth(2.5);
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(mouseClickCorners[0].x, mouseClickCorners[0].y);
    glVertex2f(mouseClickCorners[1].x, mouseClickCorners[1].y);

    glVertex2f(mouseClickCorners[1].x, mouseClickCorners[1].y);
    glVertex2f(mouseClickCorners[2].x, mouseClickCorners[2].y);

    glVertex2f(mouseClickCorners[2].x, mouseClickCorners[2].y);
    glVertex2f(mouseClickCorners[3].x, mouseClickCorners[3].y);

    glVertex2f(mouseClickCorners[3].x, mouseClickCorners[3].y);
    glVertex2f(mouseClickCorners[0].x, mouseClickCorners[0].y);

    glEnd();
  }
  glFlush();
}



/*
Keyboard Callback Routine: 'q', 'Q' or ESC quit
This routine is called every time a key is pressed on the keyboard
*/
void handleKey(unsigned char key, int x, int y)
{
  switch(key)
  {
    case 'q':		// q - quit
    case 'Q':
    case 27:		// esc - quit
      ReleasePixels();
      exit(0);
      
    default:		// not a valid key -- just ignore it
      return;
  }
}


/*
mouse callback
  left click on the output image window to get cornet positions
*/
void mouseClick(int button, int state, int x, int y)
{
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
  {
    if (mouse_index < 4)
    {
      // cout << "point" << mouse_index << ": ("<< x << ", " << y << ")" << endl;
      mouseClickCorners[mouse_index].x = x;
      mouseClickCorners[mouse_index].y = nHeightOut - y;
      cout << x << ", " << y << endl;
    }
    mouse_index++;
    if (mouse_index == 4)
    {
      // mouseClick upside down
      
      
      // do the transformation and fresh the display
      interactive();
      glutPostRedisplay();
      if (strOutputImage != "")  
      {
        writeimage(strOutputImage);
      }
    }
  }
}


/*
Reshape Callback Routine: sets up the viewport and drawing coordinates
*/
void handleReshape(int w, int h)
{
  float factor = 1;
  // make the image scale down to the largest size when user decrease the size of window
  if (w < nWidthOut || h < nHeightOut)
  {
    float xfactor = w / float(nWidthOut);
    float yfactor = h / float(nHeightOut);
    factor = xfactor;
    if (xfactor > yfactor)  {factor = yfactor;}    // fix the image shape when scale down the image size
    glPixelZoom(factor, factor);
  }
  // make the image remain centered in the window
  glViewport((w - nWidthOut * factor) / 2, (h - nHeightOut * factor) / 2, w, h);
  
  // define the drawing coordinate system on the viewport
  // to be measured in pixels
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, w, 0, h);
  glMatrixMode(GL_MODELVIEW);
}




/*
Main program
*/
int main(int argc, char* argv[])
{
  Vector2D xycorners[4];  // output image corner positions

  // command line parser and calculate transform matrix
  getCmdOptions(argc, argv, strInputImage, strOutputImage);
  // read input image
  readimage(strInputImage);

  // inverse map
  if(mode == MODE_INTERACTIVE)
  {
      nWidthOut = 1024;
      nHeightOut = 768;
      CreateOutputPixel();
      cout << "Left click the mouse in the output window to position 4 corners for output image." << endl;
      cout << "Click order: (0, 0), (0, height), (width, height), (width, 0)" << endl;
  }
  else
  {
    generateMatrix();
    switch(inversemode)
    {
        case INVERSE_LINEAR:
          boundingbox(xycorners);
          if(mode == MODE_BILINEAR)
          {
            bilinear(xycorners);
          }
          else inversemap();
          break;
        case INVERSE_TWIRL:
          nWidthOut = nWidthIn;
          nHeightOut = nHeightIn;
          CreateOutputPixel();
          AdvMap.InverseTwirl(puchOutputPixmap, puchInputPixmap, nWidthOut, nHeightOut, nWidthIn, nHeightIn);
          break;
        case INVERSE_RIPPLE:
          nWidthOut = nWidthIn;
          nHeightOut = nHeightIn;
          CreateOutputPixel();
          AdvMap.InverseRipple(puchOutputPixmap, puchInputPixmap, nWidthOut, nHeightOut, nWidthIn, nHeightIn);
          break;
    }
  }
  if (strOutputImage != "") {writeimage(strOutputImage);}
  

    
  
  // display input image and output image in seperated windows
  // start up the glut utilities
  glutInit(&argc, argv);
  // create the graphics window, giving width, height, and title text
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
  
  // display the input image
  // first window: input image
 
 
  // display the output image
  // second window: output image
  glutInitWindowSize(nWidthOut, nHeightOut);
  glutCreateWindow("Output Image");
  // set up the callback routines to be called when glutMainLoop() detects an event
  glutDisplayFunc(display);	  // display callback
  glutKeyboardFunc(handleKey);	  // keyboard callback
  if (mode == 2)  
  {
    glutMouseFunc(mouseClick);
  }  // mouse callback
  glutReshapeFunc(handleReshape); // window resize callback
  
  // Routine that loops forever looking for events. It calls the registered
  // callback routine to handle each event that is detected
  glutMainLoop();

  // release memory
  ReleasePixels();

  return 0;
}

