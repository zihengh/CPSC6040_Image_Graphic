# include <OpenImageIO/imageio.h>

# include <cstdlib>
# include <iostream>
# include <fstream>
# include <string>

# ifdef __APPLE__
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#   include <GLUT/glut.h>
# else
#   include <GL/glut.h>
# endif

using namespace std;
OIIO_NAMESPACE_USING


static string frontimagename; // frontground image file name
static string backimagename;  // background image file name
static string outfilename = "";  // output image file name
static unsigned char *puchFrontPixmap = NULL;  // frontground image pixel map
static unsigned char *puchBackPixmap = NULL; // background image pixel map
static unsigned char *puchComposePixmap = NULL; // composed image pixel map
static int nWidth = 0; // window width
static int nHeight = 0;  // window height
static int front_w = 0; // frontground image width
static int front_h = 0; // frontground image height
static int backchannels;  // background image channel number
static int posX;  // frontground image X position
static int posY;  // frontground image Y position


/*
generate associated color image
*/
void associatedColor(const unsigned char *puchPixmap, unsigned char *associatedpuchPixmap, int w, int h)
{
  for (int nCol = 0; nCol < w; nCol++)
  {
    for (int nRow = 0; nRow < h; nRow++)
    {
      float r, g, b, alpha, a;
      r = puchPixmap[(nRow * w + nCol) * 4];
      g = puchPixmap[(nRow * w + nCol) * 4 + 1];
      b = puchPixmap[(nRow * w + nCol) * 4 + 2];
      alpha = puchPixmap[(nRow * w + nCol) * 4 + 3];

      a = float(alpha) / 255;
      associatedpuchPixmap[(nRow * w + nCol) * 4] = float(r) * a;
      associatedpuchPixmap[(nRow * w + nCol) * 4 + 1] = float(g) * a;
      associatedpuchPixmap[(nRow * w + nCol) * 4 + 2] = float(b) * a;
      associatedpuchPixmap[(nRow * w + nCol) * 4 + 3] = alpha;
    }
  }
}


/*
over operation
  front value, back value and alpha value on scale 0-255
*/
int over(int front, int back, int alpha)
{
  int composedValue;
  composedValue = float(front) + (1 - (float(alpha) / 255)) * float(back);

  return composedValue;
}


/*
composition
  convert frontground image to associated color image (no need to do so for background image due to 255 alpha value)
  and do the over operation between associated frontground image and background image
*/
void compose(unsigned char *puchFrontPixmap, unsigned char *puchBackPixmap, int posX, int posY)
{
  puchComposePixmap = new unsigned char [nWidth * nHeight * 4];
  for (int nCol = 0; nCol < nWidth; nCol++)
  {
    for (int nRow = 0; nRow < nHeight; nRow++)
    {
      // get associated frontground image pixel value
      float frontR = 0;
      float frontG = 0;
      float frontB = 0;
      float frontA = 0;
      if (nCol >= posX && nCol < posX + front_w && nRow >= posY && nRow < posY + front_h)
      {
        int x, y;
        x = nCol - posX;
        y = nRow - posY;
        frontR = puchFrontPixmap[(y * front_w + x) * 4];
        frontG = puchFrontPixmap[(y * front_w + x) * 4 + 1];
        frontB = puchFrontPixmap[(y * front_w + x) * 4 + 2];
        frontA = puchFrontPixmap[(y * front_w + x) * 4 + 3];
      }

      // get associated background image pixel value (background image alpha = 255)
      float backR, backG, backB;
      backR = puchBackPixmap[(nRow * nWidth + nCol) * backchannels];
      backG = puchBackPixmap[(nRow * nWidth + nCol) * backchannels + 1];
      backB = puchBackPixmap[(nRow * nWidth + nCol) * backchannels + 2];

      // over operation for each channel value and set alpha value as 255
      puchComposePixmap[(nRow * nWidth + nCol) * 4] = over(frontR, backR, frontA);
      puchComposePixmap[(nRow * nWidth + nCol) * 4 + 1] = over(frontG, backG, frontA);
      puchComposePixmap[(nRow * nWidth + nCol) * 4 + 2] = over(frontB, backB, frontA);
      puchComposePixmap[(nRow * nWidth + nCol) * 4 + 3] = 255;
    }
  }
}


/*
get the image puchPixmap
*/
void readimage(string infilename, bool backflag)
{
  // read the input image and store as a puchPixmap
  ImageInput *in = ImageInput::open(infilename);
  if (!in)
  {
    cerr << "Cannot get the input image for " << infilename << ", error = " << geterror() << endl;
    exit(0);
  }
  // get the image size and channels information, allocate space for the image
    const ImageSpec &spec = in -> spec();
    int w, h, channels;
    w = spec.width;
    h = spec.height;
    channels = spec.nchannels;
    int nScanlineSize = w * channels * sizeof(unsigned char);

    // background image
    if (backflag)
    {
      nWidth = w;
      nHeight = h;
      backchannels = channels;
      if (channels < 4)
      {
        cout << "backgroundimage should have 4 channels." << endl;
//        exit(0);
      }
      cout << "starting to read background image" << endl;
      puchBackPixmap = new unsigned char [w * h * channels];
      if(!in -> read_image(TypeDesc::UINT8, puchBackPixmap+(h-1)*nScanlineSize, AutoStride, -nScanlineSize))
      {
        cerr << "Could not read Input image for " << infilename << ", error = " << geterror() << endl;
      }
      in -> close();  // close the file
      ImageInput::destroy(in);
      return ;
    }
    // frontground image
    front_w = w;
    front_h = h;
    unsigned char puchPixmap[w * h * channels] = {0};
    if (channels < 4)
    {
        cout << "Frontimage should have 4 channels." << endl;
        exit(0);
    }

    puchFrontPixmap = new unsigned char [w * h * 4];
    in -> read_image(TypeDesc::UINT8, puchPixmap +(h-1)*nScanlineSize, AutoStride, -nScanlineSize);
    associatedColor(puchPixmap, puchFrontPixmap, w, h);
    in -> close();  // close the file
    ImageInput::destroy(in);
}


/*
write out the associated color image from image pixel map
*/



/*
display composed associated color image
*/
void display()
{
  // modify the puchPixmap: upside down the image and add A channel value for the image

  // display the puchPixmap
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glRasterPos2i(0, 0);
  // glDrawPixels writes a block of pixels to the framebuffer.
  glDrawPixels(nWidth, nHeight, GL_RGBA, GL_UNSIGNED_BYTE, puchComposePixmap);
  glFlush();
}


/*
Routine to get image from OpenGL framebuffer and then write the associated composed image to an image file
*/
void writeglimage()
{
  // get the output file name
  if(outfilename == "")
  {
    cout << "Enter output image filename: ";
    cin >> outfilename;
  }



  // create the subclass instance of ImageOutput which can write the right kind of file format
  ImageOutput *out = ImageOutput::create(outfilename);
  if (!out)
  {
    cerr << "Could not create output image for " << outfilename << ", error = " << geterror() << endl;
  }
  int nScanlineSize = nWidth * 4 * sizeof(unsigned char);

    ImageSpec spec (nWidth, nHeight, 4, TypeDesc::UINT8);
    out -> open(outfilename, spec);

    out -> write_image(TypeDesc::UINT8, puchComposePixmap + (nHeight-1)*nScanlineSize, AutoStride, -nScanlineSize);
    cout << "Write the image puchPixmap to image file " << outfilename << endl;

    out -> close();

    delete out;
}



void handleKey(unsigned char key, int x, int y)
{
  switch(key)
  {
    // write the current window to an image file
    case 'w':
    case 'W':
      writeglimage();
      break;
    // quit the program
    case 'q':		// q - quit
    case 'Q':
    case 27:		// esc - quit
      exit(0);

    default:		// not a valid key -- just ignore it
      return;
  }
}

void handleSpecialKey(int nKey, int x, int y)
{
    switch(nKey)
  {
    case GLUT_KEY_LEFT:
        posX = posX - (nWidth - front_w)/20;
        if (posX + front_w <= 0)  {posX = nWidth;}
        compose(puchFrontPixmap, puchBackPixmap, posX, posY);
        display();
        break;

    case GLUT_KEY_RIGHT:
        posX = posX + (nWidth - front_w)/20;
        if (posX > nWidth)  {posX = -front_w;}
        compose(puchFrontPixmap, puchBackPixmap, posX, posY);
        display();
        break;
    case GLUT_KEY_DOWN:
        posY = posY - (nHeight - front_h)/20;
        if (posY + front_h< 0)  {posY = nHeight;}
        compose(puchFrontPixmap, puchBackPixmap, posX, posY);
        display();
        break;
    case GLUT_KEY_UP:
        posY = posY + (nHeight - front_h)/20;
        if (posY > nHeight)  {posY = -front_h;}
        compose(puchFrontPixmap, puchBackPixmap, posX, posY);
        display();
        break;
    default:		// not a valid key -- just ignore it
      return;
  }
}

/*
Reshape Callback Routine: sets up the viewport and drawing coordinates
*/
void handleReshape(int w, int h)
{
  float factor = 1;
  // make the image scale down to the largest size when user decrease the size of window
  if (w < nWidth || h < nHeight)
  {
    float xfactor = w / float(nWidth);
    float yfactor = h / float(nHeight);
    factor = xfactor;
    if (xfactor > yfactor)  {factor = yfactor;}    // fix the image shape when scale down the image size
    glPixelZoom(factor, factor);
  }
  // make the image remain centered in the window
  glViewport((w - nWidth * factor) / 2, (h - nHeight * factor) / 2, w, h);

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
  // command line: get frontground image and background image
  // usage: compose <frontimagename> <backimagename> (optional)<outputfilename>
  if (argc >= 3)  // one argument of image file name
  {
    frontimagename = argv[1];
    backimagename = argv[2];
    if (argc > 3)
      {
        cout << "Output image file name: " << argv[3] << endl;
        outfilename = argv[3];
      }
  }
  else
  {
    cout << "[Usage] compose <frontimagename> <backimagename> (optional)<outputfilename>" << endl;
    return 0;
  }

  readimage(backimagename, 1);
  // read input image

  readimage(frontimagename, 0);


  // set frontground image position as middle
  posX = float(nWidth - front_w) / 2;
  posY = nHeight - front_h;
  // compose frountground image with background image
  cout << "Compose images..." << endl;
  compose(puchFrontPixmap, puchBackPixmap, posX, posY);


  int w = nWidth;
  int h = nHeight;

  // start up the glut utilities
  glutInit(&argc, argv);

  // create the graphics window, giving width, height, and title text
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
  glutInitWindowSize(w, h);
  glutCreateWindow("Composed Dr.House Image");

  // set up the callback routines to be called when glutMainLoop() detects an event
  glutDisplayFunc(display);	  // display callback
  glutKeyboardFunc(handleKey);	  // keyboard callback
  glutReshapeFunc(handleReshape); // window resize callback
  glutSpecialFunc(handleSpecialKey);
  // Routine that loops forever looking for events. It calls the registered
  // callback routine to handle each event that is detected
  glutMainLoop();

  delete []puchFrontPixmap;
  delete []puchBackPixmap;
  delete []puchComposePixmap;

  return 0;
}

