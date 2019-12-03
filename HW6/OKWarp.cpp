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
# include "invmap.h"

# ifdef __APPLE__
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#   include <GLUT/glut.h>
# else
#   include <GL/glut.h>
# endif

using namespace std;
OIIO_NAMESPACE_USING
static string strOutputFile = "";
static unsigned char *puchInputPixmap = NULL;  // input image pixels 
static unsigned char *puchReconstructedPixmap = NULL;
static unsigned char *puchWarpedPixmap = NULL; // warped image pixels
static unsigned char *puchRepairedPixmap = NULL; // repaired image pixels
static int nWidthIn, nHeightIn;  // input image size: width, height
static int nWidthOut, nHeightOut;  // output image size: width, height
static int invmode = 0xff;
static int repairmode = 0xff;
InvMap cInvMap;


void CreateOutputPixel()
{
  if(puchWarpedPixmap != NULL)
  {
    delete []puchWarpedPixmap;
    puchWarpedPixmap = NULL;
  }
  puchWarpedPixmap = new unsigned char[nWidthOut*nHeightOut*4];
  memset(puchWarpedPixmap, 0, nWidthOut*nHeightOut*4*sizeof(unsigned char));

  if(puchRepairedPixmap != NULL)
  {
    delete []puchRepairedPixmap;
    puchRepairedPixmap = NULL;
  }
  puchRepairedPixmap = new unsigned char[nWidthOut*nHeightOut*4];
  memset(puchRepairedPixmap, 0, nWidthOut*nHeightOut*4*sizeof(unsigned char));
}

void ReleasePixels()
{
  if(puchRepairedPixmap != NULL)
  {
    delete []puchRepairedPixmap;
    puchRepairedPixmap=NULL;
  }
  if(puchWarpedPixmap != NULL)
  {
    delete []puchWarpedPixmap;
    puchWarpedPixmap=NULL;
  }
  if(puchReconstructedPixmap != NULL)
  {
    delete []puchReconstructedPixmap;
    puchReconstructedPixmap=NULL;
  }
  if(puchInputPixmap != NULL)
  {
    delete []puchInputPixmap;
    puchInputPixmap=NULL;
  }
}





void readimage(string infilename)
{
  // read the input image and store as a pixmap
  ImageInput *in = ImageInput::open(infilename);
  if (!in)
  {
    cerr << "Cannot get the input image for " << infilename << ", error = " << geterror() << endl;
    exit(0);
  }
  // get the image size and channels information, allocate space for the image
    const ImageSpec &spec = in -> spec();

    nWidthIn = spec.width;
    nHeightIn = spec.height;
    int channels = spec.nchannels;
    cout << "Input image size: " << nWidthIn << "x" << nHeightIn << endl;
    cout << "channels: " << channels << endl;

    unsigned char tmppixmap[nWidthIn * nHeightIn * channels];
    int nScanlineSize = nWidthIn * channels * sizeof(unsigned char);
    if(!in -> read_image(TypeDesc::UINT8, tmppixmap+(nHeightIn-1)*nScanlineSize, AutoStride, -nScanlineSize))
    {
        cerr << "Could not read input image for " << infilename << ", error = " << geterror() << endl;
    }

    
    if(puchInputPixmap!=NULL)
    {
      delete []puchInputPixmap;
      puchInputPixmap = NULL;
    }
    puchInputPixmap = new unsigned char[nWidthIn*nHeightIn*4];
    memset(puchInputPixmap, 0, nWidthIn*nHeightIn*4*sizeof(unsigned char));

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
            {
                puchInputPixmap[(row * nWidthIn + col) * 4 + k] = tmppixmap[(row * nWidthIn + col) * 3 + k];
            }
            puchInputPixmap[(row * nWidthIn + col) * 4 + 3] = 255;
            break;
          case 4:
            for (int k = 0; k < 4; k++)
            {
                puchInputPixmap[(row * nWidthIn + col) * 4 + k] = tmppixmap[(row * nWidthIn + col) * 4 + k];
            }
            break;
          default:
            in -> close();  // close the file
            delete in;    // free ImageInput
            return;
        }
      }
    }


    in -> close();  // close the file
    delete in;    // free ImageInput
  
}

/*
write out the associated color image from image pixel map
*/
void writeimage(string outfilename, unsigned char* puchPixmap)
{   
  // create the subclass instance of ImageOutput which can write the right kind of file format
  ImageOutput *out = ImageOutput::create(outfilename);
  if (!out)
  {
    cerr << "Could not create output image for " << outfilename << ", error = " << geterror() << endl;
    ReleasePixels();
    exit(2) ;
  }
  

    // .ppm file: 3 channels
  if (outfilename.substr(outfilename.find_last_of(".") + 1, outfilename.length() - 1) == "ppm")
  {
    unsigned char pixmap[nWidthOut * nHeightOut * 3] = {0};
    for (int row = 0; row < nHeightOut; row++)
    {
      for (int col = 0; col < nWidthOut; col++)
      {
        for (int k = 0; k < 3; k++)
        {
            pixmap[(row * nWidthOut + col) * 3 + k] = puchPixmap[(row * nWidthOut + col) * 4 + k];
        }
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
  out -> write_image(TypeDesc::UINT8, puchPixmap+(nHeightOut-1)*nScanlineSize, AutoStride, -nScanlineSize);

  
  out -> close();
  delete out;
}

void Reconstruct()
{
  if(puchReconstructedPixmap!=NULL)
    {
      delete []puchReconstructedPixmap;
      puchReconstructedPixmap = NULL;
    }
  puchReconstructedPixmap = new unsigned char[nWidthIn*nHeightIn*4];
  memset(puchReconstructedPixmap, 0, nWidthIn*nHeightIn*4*sizeof(unsigned char));
  for (int row_in = 0; row_in < nHeightIn; row_in++)
  {
    for (int col_in = 0; col_in < nWidthIn; col_in++)
      {
        if(repairmode == MODE_REPAIR_SUPERSAMPLING)
        {
            cInvMap.supersampling(row_in, col_in, puchReconstructedPixmap, puchInputPixmap);  
        } 
        else cInvMap.ad_supersampling(row_in, col_in, puchReconstructedPixmap, puchInputPixmap);
      }
  }
}

void RepairPixel(float u, float v, int row_out, int col_out, double scale_factor_x, double scale_factor_y, int channel)
{
    int row_in = floor(v);
    int col_in = floor(u);
    switch(repairmode)
    {
        case MODE_REPAIR_BILINEAR:
          if (scale_factor_x < 1 || scale_factor_y < 1)
          {
            puchRepairedPixmap[(row_out * nWidthOut + col_out) * 4 + channel] = cInvMap.bilinear_interpolation(u, v, row_out, col_out, channel, puchInputPixmap);
          }
          else puchRepairedPixmap[(row_out * nWidthOut + col_out) * 4 + channel] = puchInputPixmap[(row_in * nWidthIn + col_in) * 4 + channel];
          break;
        case MODE_REPAIR_SUPERSAMPLING:
        case MODE_REPAIR_AD_SUPERSAMPLE:
          if (scale_factor_x > 1 || scale_factor_y > 1)
          {
            puchRepairedPixmap[(row_out * nWidthOut + col_out) * 4 + channel] = puchReconstructedPixmap[(row_in * nWidthIn + col_in) * 4 + channel];
          }
          else puchRepairedPixmap[(row_out * nWidthOut + col_out) * 4 + channel] = puchInputPixmap[(row_in * nWidthIn + col_in) * 4 + channel];
          break;
        case MODE_REPAIR_AUTO:
         if (scale_factor_x < 1 || scale_factor_y < 1)
         {
            if (scale_factor_x < 1 && scale_factor_y < 1)
            {
                puchRepairedPixmap[(row_out * nWidthOut + col_out) * 4 + channel] = cInvMap.bilinear_interpolation(u, v, row_out, col_out, channel, puchInputPixmap);
            }
            else puchRepairedPixmap[(row_out * nWidthOut + col_out) * 4 + channel] = cInvMap.bilinear_interpolation(u, v, row_out, col_out, channel, puchReconstructedPixmap);
         }
         else if (scale_factor_x == 1 && scale_factor_y == 1)
         {
             puchRepairedPixmap[(row_out * nWidthOut + col_out) * 4 + channel] = puchInputPixmap[(row_in * nWidthIn + col_in) * 4 + channel];
         }
         else puchRepairedPixmap[(row_out * nWidthOut + col_out) * 4 + channel] = puchReconstructedPixmap[(row_in * nWidthIn + col_in) * 4 + channel];
         break;
        default:
            return ;

    }
}





/*
warp image
*/
void warpimage()
{
  // inverse map
  float x, y, u, v;
  
  for (int row_out = 0; row_out < nHeightOut; row_out++)  // output row
  {
    for (int col_out = 0; col_out < nWidthOut; col_out++)  // output col
    {
      double scale_factor_x = 1.0;
      double scale_factor_y = 1.0;
      x = float(col_out) + 0.5;
      y = float(row_out) + 0.5;      

      cInvMap.scale_factor(x, y, scale_factor_x, scale_factor_y);
      // inverse mapping functions
      cInvMap.InverseMap(x, y, u, v);

      if (u < nWidthIn && v < nHeightIn && u >= 0 && v >= 0)
      {
        int row_in, col_in;
        row_in = floor(v);
        col_in = floor(u);
        for (int k = 0; k < 4; k++)
        {
          puchWarpedPixmap[(row_out * nWidthOut + col_out) * 4 + k] = puchInputPixmap[(row_in * nWidthIn + col_in) * 4 + k];

          RepairPixel( u,  v,  row_out,  col_out,  scale_factor_x,  scale_factor_y,  k);
        }
      }
    }
  }
}

/*
display composed associated color image
*/
void display(const unsigned char *pixmap)
{
  if(pixmap == NULL)
  {
      return;
  }
  // display the pixmap
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glRasterPos2i(0, 0);
  // glDrawPixels writes a block of pixels to the framebuffer
  glDrawPixels(nWidthOut, nHeightOut, GL_RGBA, GL_UNSIGNED_BYTE, pixmap);

  glFlush();
}

void displayWarped() 
{
    display(puchWarpedPixmap);
}

void displayRepaired()  
{
    display(puchRepairedPixmap);
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


void handleWarpedKey(unsigned char key, int x, int y)
{
  switch(key)
  {
    case 'w':		
    case 'W':
      cout <<  "type in the image name you want to store:" << endl;
      
      cin >> strOutputFile;
      writeimage(strOutputFile, puchWarpedPixmap);
      cout <<  "warped image stored" << endl;
      break;
    case 'q':		// q - quit
    case 'Q':
    case 27:		// esc - quit
      ReleasePixels();
      exit(0);
      break;
    default:		// not a valid key -- just ignore it
      return;
  }
}

void handleRepairedKey(unsigned char key, int x, int y)
{
  switch(key)
  {
    case 'w':		
    case 'W':
      cout <<  "type in the image name you want to store:" << endl;
      cin >> strOutputFile;
      writeimage(strOutputFile, puchRepairedPixmap);
      cout <<  "repaired image stored" << endl;
      break;
    case 'q':		// q - quit
    case 'Q':
    case 27:		// esc - quit
      ReleasePixels();
      exit(0);
      break;
    default:		// not a valid key -- just ignore it
      return;
  }
}

void getCmdOptions(int argc, char* argv[], string &inputImage)
{
  if (argc < 3)
  {
    return;
    exit(1);
    
  }
    // get input image name
  inputImage = argv[1];
  invmode = atof(argv[2]);
  repairmode = atof(argv[3]);
//  cout << "warp function: " << invmode << endl;
 
}

/*
Main program
*/
int main(int argc, char* argv[])
{
  string inputImage;  // input image file name
  string outputImage; // output image file name

  // command line parser
  getCmdOptions(argc, argv, inputImage);

  // read input image
  readimage(inputImage);

  nWidthOut = nWidthIn;
  nHeightOut = nHeightIn;

  cout << "Output image size: " << nWidthOut << "x" << nHeightOut << endl;

  
  // allocate memory for output pixmap: output image is 4 channels
  CreateOutputPixel();

  // warp the original image
  cInvMap.SetImageInfo(invmode, repairmode, nWidthIn, nHeightIn, nWidthOut, nHeightOut);
  Reconstruct(); 
  warpimage();

  
  
  // display input image and output image in seperated windows
  // start up the glut utilities
  glutInit(&argc, argv);
  
  // create the graphics window, giving width, height, and title text
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);

  // first window: warped image
  glutInitWindowSize(nWidthOut, nHeightOut);
  glutCreateWindow("warped Image");  
  glutDisplayFunc(displayWarped);	  // display callback
  glutReshapeFunc(handleReshape); // window resize callback
  glutKeyboardFunc(handleWarpedKey);	  // keyboard callback
 
  // second window: repaired image
  glutInitWindowSize(nWidthOut, nHeightOut);
  glutCreateWindow("repaired Image");  
  glutDisplayFunc(displayRepaired);	  // display callback
  glutReshapeFunc(handleReshape); // window resize callback
  glutKeyboardFunc(handleRepairedKey);	  // keyboard callback

  glutMainLoop();

  // release memory
  ReleasePixels();

  return 0;
}

