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

//static string infilename; // input image name
//static string outfilename;  // output image name
static int nWidth;  // input image width
static int nHeight;  // input image height
static unsigned char *puchPixmap = NULL;  // output image pixel map
static unsigned char *puchOutPixmap = NULL;  // output image pixel map

double db_hl = 80;
double db_hh = 150;
double db_sl = 0.4;
double db_vl = 0.3;
double db_sh = 1;
double db_vh = 1;
double db_ghl = 40;
double db_ghh = 180;
double db_spill_s = 0.5;
double db_spill_k = 0.5;
unsigned char uchValMode = 0;
# define maximum(x, y, z) ((x) > (y)? ((x) > (z)? (x) : (z)) : ((y) > (z)? (y) : (z)))
# define minimum(x, y, z) ((x) < (y)? ((x) < (z)? (x) : (z)) : ((y) < (z)? (y) : (z)))
enum
{
    MODE_H=0,
    MODE_S,
    MODE_V,
    MODE_GH,
    MODE_SPILL,
};

/*
convert RGB color image to HSV color image
  input RGB color primary values: r, g, and b on scale 0 - 255
  Output HSV colors: h on scale 0-360, s and v on scale 0-1
*/
void RGBtoHSV(int r, int g, int b, double &h, double &s, double &v)
{
  double red, green, blue;
  double max, min, delta;

  red = r / 255.0; green = g / 255.0; blue = b / 255.0;  /* r, g, b to 0 - 1 scale */

  max = maximum(red, green, blue);
  min = minimum(red, green, blue);

  v = max;        /* value is maximum of r, g, b */

  if(max == 0){    /* saturation and hue 0 if value is 0 */
     s = 0;
     h = 0;
  }
  else{
    s = (max - min) / max;           /* saturation is color purity on scale 0 - 1 */

    delta = max - min;
    if(delta == 0)                         /* hue doesn't matter if saturation is 0 */
       h = 0;
    else{
       if(red == max)                    /* otherwise, determine hue on scale 0 - 360 */
          h = (green - blue) / delta;
      else if(green == max)
          h = 2.0 + (blue - red) / delta;
      else /* (green == max) */
          h = 4.0 + (red - green) / delta;

      h = h * 60.0;                       /* change hue to degrees */
      if(h < 0)
          h = h + 360.0;                /* negative hue rotated to equivalent positive around color wheel */
    }
  }
}


void get_thresholds()
{
  fstream thresholdsFile("thresholds.txt");
thresholdsFile >> db_hl >> db_hh >> db_sl >> db_sh >> db_vl >> db_vh >> db_ghl >> db_ghh >> db_spill_s >> db_spill_k;
}

void save_thresholds()
{
  fstream thresholdsFile("thresholds.txt");
thresholdsFile << db_hl << '\n' << db_hh << '\n' << db_sl<< '\n' << db_sh<< '\n' << db_vl << '\n' << db_vh << '\n' << db_ghl<< '\n' << db_ghh << '\n' << db_spill_s << '\n' <<  db_spill_k << '\n';
cout << "threshold parameter file has been saved" << endl;
}
/*
get the image puchPixmap
*/
void readimage(string infilename)
{
  // read the input image and store as a puchPixmap
  ImageInput *in = ImageInput::open(infilename);
  if (!in)
  {
    cerr << "Cannot get the input image for " << infilename << ", error = " << geterror() << endl;
    return ;
  }
  // get the image size and channels information, allocate space for the image
    const ImageSpec &spec = in -> spec();
    int channels;
    nWidth = spec.width;
    nHeight = spec.height;
    channels = spec.nchannels;
//    unsigned char uchTempPixel[nWidth * nHeight * channels] = {0};
    puchPixmap = new unsigned char [nWidth * nHeight * channels];
    int nScanlineSize = nWidth * channels * sizeof(unsigned char);
    if(!in -> read_image(TypeDesc::UINT8, puchPixmap+(nHeight-1)*nScanlineSize, AutoStride, -nScanlineSize))
    {
        cerr << "Could not read input image for " << infilename << ", error = " << geterror() << endl;
    }
//    if(!in -> read_image(TypeDesc::UINT8, puchPixmap))
//    {
//        cerr << "Could not read input image for " << infilename << ", error = " << geterror() << endl;
//    }
    in -> close();  // close the file
    ImageInput::destroy(in);
      // free imageinput
    cout << "image read successfully" << endl;
    get_thresholds();
//    memcpy(puchPixmap, uchTempPixel, nWidth * nHeight * channels*sizeof(unsigned char));
}


/*
write out the mask image
*/
void writeimage(string outfilename)
{
  if(outfilename == "")
  {
    cout << "type the name of the imagefile you want to store:";
    cin >> outfilename;
  }
  // create the subclass instance of imageOutput which can write the right kind of file format
  ImageOutput *out = ImageOutput::create(outfilename);
  if (!out)
  {
    cerr << "Could not create output image for " << outfilename << ", error = " << geterror() << endl;
  }
  else
  {
    // open and prepare the image file
    ImageSpec spec (nWidth, nHeight, 4, TypeDesc::UINT8);
    out -> open(outfilename, spec);
    // write the entire image
    int nScanlineSize = nWidth * 4 * sizeof(unsigned char);
    out -> write_image(TypeDesc::UINT8, puchOutPixmap + (nHeight-1)*nScanlineSize, AutoStride, -nScanlineSize);
    cout <<  outfilename << " has been stored" << endl;
    // close the file and free the imageOutput i created
    out -> close();

    delete out;
  }
}

bool IsInRange(double dbVal, double dbL, double dbH)
{
    if(dbVal < dbL)
    {
        return false;
    }
    if(dbVal > dbH)
    {
        return false;
    }
    return true;
}

unsigned char GradientEdge(double dbVal, double dbLL, double dbL, double dbH, double dbHH)
{
    if(!IsInRange(dbVal, dbLL, dbHH))
    {
        return 255;
    }
    if (dbVal <= dbL)
    {
        return (dbVal / (dbL - dbLL)) * 255;
    }
    return (dbVal / (dbHH - dbH)) * 255;

}



void alphamask()
{
  char chszThreshold[512] = {0};
  // get thresholds
  sprintf(chszThreshold, "h:%f~%f\ns:%f~%f\nv:%f~%f", db_hl, db_hh, db_sl, db_sh, db_vl, db_vh);
  cout << chszThreshold << endl;
  sprintf(chszThreshold, "k1:%f  k2:%f", db_spill_s, db_spill_k);
  cout << chszThreshold << endl;
  // convert rgb color to hsv color
  puchOutPixmap = new unsigned char [nWidth * nHeight * 4];
  for (int nCol = 0; nCol < nWidth; nCol++)
  {
    for (int nRow = 0; nRow < nHeight; nRow++)
    {
      int r, g, b;
      double h, s, v;
      unsigned char alpha;
      r = puchPixmap[(nRow * nWidth + nCol) * 3];
      g = puchPixmap[(nRow * nWidth + nCol) * 3 + 1];
      b = puchPixmap[(nRow * nWidth + nCol) * 3 + 2];


      RGBtoHSV(r, g, b, h, s, v);
      if(g > db_spill_s*r + db_spill_k*b)
      {
        g = db_spill_s*r + db_spill_k*b;
      }


      puchOutPixmap[(nRow * nWidth + nCol) * 4] = r;
      puchOutPixmap[(nRow * nWidth + nCol) * 4 + 1] = g;
      puchOutPixmap[(nRow * nWidth + nCol) * 4 + 2] = b;
      // generate alpha channel mask
      alpha = 255;

      if(!IsInRange(h, db_hl, db_hh))
      {
        puchOutPixmap[(nRow * nWidth + nCol) * 4 + 3] = GradientEdge(h, db_ghl, db_hl, db_hh, db_ghh) ;
//        puchOutPixmap[(nRow * nWidth + nCol) * 4 + 3] = 255 ;
        continue;
      }
      if(!IsInRange(s, db_sl, db_sh))
      {
        puchOutPixmap[(nRow * nWidth + nCol) * 4 + 3] = GradientEdge(h, db_ghl, db_hl, db_hh, db_ghh);
 //       puchOutPixmap[(nRow * nWidth + nCol) * 4 + 3] = 255 ;
        continue;
      }
      if(!IsInRange(v, db_vl, db_vh))
      {
        puchOutPixmap[(nRow * nWidth + nCol) * 4 + 3] = GradientEdge(h, db_ghl, db_hl, db_hh, db_ghh);
  //      puchOutPixmap[(nRow * nWidth + nCol) * 4 + 3] = 255 ;
        continue;
      }

      puchOutPixmap[(nRow * nWidth + nCol) * 4 + 3] = 0;
    }
  }
}






void GetimageRatio(int nImgWidth, int nImgHeight, int nWindowWidth, int nWindowHeight, float &fRatio)
{
    if(nImgWidth<nWindowWidth && nImgHeight < nWindowHeight)
    {
        return ;
    }
    if(nImgWidth * nWindowHeight > nImgHeight * nWindowWidth)
    {
        fRatio = ((float)nWindowWidth) / ((float)nImgWidth) ;
        return ;
    }
    if(nImgWidth * nWindowHeight <= nImgHeight * nWindowWidth)
    {
        fRatio =  ((float)nWindowHeight)/ ((float)nImgHeight);
        return ;
    }
}

void handleReShapeWindow(int iW, int iH)
{
    glViewport(0, 0, iW, iH);

	// define the drawing coordinate system on the viewport
//	glMatrixMode(GL_PROJECTiON);
	glLoadIdentity();
	gluOrtho2D(0, iW, 0, iH);// sets up a 2D orthographic viewing region
//	Displayimage();
}



void display()
{
  glMatrixMode ( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  glClearColor(1, 1, 1, 0);
  char chszThreshold[1024] = {0};
  glColor3f ( 0, 0, 0 );
  glRasterPos2i ( 10, 128 );

  cout << chszThreshold << endl;
  glPopMatrix();
  glMatrixMode ( GL_MODELVIEW );

  if(puchOutPixmap == NULL)
  {
    glEnd();
    glFlush();
    return ;
  }
  int nWindowWidth=glutGet(GLUT_WINDOW_WIDTH);
  int nWindowHeight = glutGet(GLUT_WINDOW_HEIGHT);
  // display the puchPixmap
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  float fRatio = 1;
  GetimageRatio(nWidth, nHeight, nWindowWidth, nWindowHeight, fRatio);
  glRasterPos2i(0.5*nWindowWidth-0.5*nWidth*fRatio, 0.5*nWindowHeight-0.5*nHeight*fRatio);

  glPixelZoom(fRatio, fRatio);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  // glDrawPixels writes a block of pixels to the framebuffer.
  glDrawPixels(nWidth, nHeight, GL_RGBA, GL_UNSIGNED_BYTE, puchOutPixmap);
  glDisable(GL_BLEND);
  glEnd();
  glFlush();
}

void AdjustHSVRange(bool bUpperBound, bool bIncrease)
{
    switch(uchValMode)
    {
        case MODE_H:
            if(bUpperBound)
            {
                if(bIncrease)
                {
                    db_hh = db_hh + 1;
                    break;
                }
                db_hh = db_hh -1;
                break;
            }
            if(bIncrease)
            {
                db_hl = db_hl + 1;
                break;
            }
            db_hl = db_hl -1;
            break;
        case MODE_S:
            if(bUpperBound)
            {
                if(bIncrease)
                {
                    db_sh = db_sh + 0.01;
                    break;
                }
                db_sh = db_sh -0.01;
                break;
            }
            if(bIncrease)
            {
                db_sl = db_sl + 0.01;
                break;
            }
            db_sl = db_sl -0.01;
            break;
        case MODE_V:
            if(bUpperBound)
            {
                if(bIncrease)
                {
                    db_vh = db_vh + 0.01;
                    break;
                }
                db_vh = db_vh -0.01;
                break;
            }
            if(bIncrease)
            {
                db_vl = db_vl + 0.01;
                break;
            }
            db_vl = db_vl -0.01;
            break;
        case MODE_GH:
            if(bUpperBound)
            {
                if(bIncrease)
                {
                    db_ghh = db_ghh + 1;
                    break;
                }
                db_ghh = db_ghh -1;
                break;
            }
            if(bIncrease)
            {
                db_ghl = db_ghl + 1;
                break;
            }
            db_ghl = db_ghl -1;
            break;
        case MODE_SPILL:
            if(bUpperBound)
            {
                if(bIncrease)
                {
                    db_spill_s = db_spill_s + 0.01;
                    break;
                }
                db_spill_s = db_spill_s -0.01;
                break;
            }
            if(bIncrease)
            {
                db_spill_k = db_spill_k + 0.01;
                break;
            }
            db_spill_k = db_spill_k -0.01;
            break;
    }
    alphamask();
    display();
}

void handleSpecialKey(int nKey, int x, int y)
{
    switch(nKey)
  {
    case GLUT_KEY_LEFT:
        AdjustHSVRange(false, false);
        break;
    case GLUT_KEY_RIGHT:
        AdjustHSVRange(false, true);
        break;
    case GLUT_KEY_UP:
        AdjustHSVRange(true, true);
        break;
    case GLUT_KEY_DOWN:
        AdjustHSVRange(true, false);
        break;
    default:		// not a valid key -- just ignore it
      return;
  }
}

void handleKey(unsigned char key, int x, int y)
{
  switch(key)
  {
    // write the current window to an image file
    case '1':
        uchValMode = MODE_H;
        break;
    case '2':
        uchValMode = MODE_S;
        break;
    case '3':
        uchValMode = MODE_V;
        break;
    case '4':
        uchValMode = MODE_GH;
        break;
    case '5':
        uchValMode = MODE_SPILL;
        break;
    case 'w':
    case 'W':
      writeimage("");
      break;
    case 's':
    case 'S':
      save_thresholds();
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
/*
Main program
*/
int main(int argc, char* argv[])
{
  // command line: get inputfilename and outputfilename
  // usage: alphamask <inputfilename> <outputfilename>
  if (argc <2)
  {
     cout << "No Green screen image to input" << endl;
     return 0;
  }

  string infilename = argv[1];
  // read input image

  readimage(infilename);
  // generate alpha channel mask

  alphamask();

  if(argc >=3)
  {
    writeimage(argv[2]);
  }
  // write out the image
//  cout << "Write image..." << endl;
//  writeimage(outfilename);
  glutInit(&argc, argv);

  // create the graphics window, giving width, height, and title text
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
  glutInitWindowSize(nWidth, nHeight);
  glutCreateWindow("GreenScreen");

  // set up the callback routines to be called when glutMainLoop() detects an event
  glutDisplayFunc(display);	  // display callback
  glutKeyboardFunc(handleKey);	  // keyboard callback
  glutReshapeFunc(handleReShapeWindow); // window resize callback
  glutSpecialFunc(handleSpecialKey);
//
//  glutReshapeFunc(handleReshape); // window resize callback
  glutMainLoop();

  delete []puchPixmap;
  delete []puchOutPixmap;

  return 0;
}

