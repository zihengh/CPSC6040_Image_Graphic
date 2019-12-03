# include <OpenImageIO/imageio.h>
# include <stdlib.h>
# include <iostream>
# include <fstream>
# include <string>
# include <algorithm>
# include <math.h>
# include <cmath>
# include <iomanip>


# ifdef __APPLE__
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#   include <GLUT/glut.h>
# else
#   include <GL/glut.h>
# endif

#include "convolution.h"

enum
{
  MODE_GARBOR = 0,
  MODE_FILTFILE,
};

using namespace std;
OIIO_NAMESPACE_USING
Convolution cConvolution;
static string strInputImgName;
static string strOutputImgName = "";
static float *pfPixmap = NULL;   // output image pixel map

static int nChannels;   // color channel number of input image
static int nWidth, nHeight;   // window size: image width, image height

void CreateOutputPixels(int nWidth, int nHeight, int nChannels)
{
    if(pfPixmap !=NULL)
    {
        delete []pfPixmap;
    }
    pfPixmap = new float [nWidth * nHeight * nChannels];

}

void ReleasePixmap()
{
    if(pfPixmap ==NULL)
    {
        return;
    }
    delete []pfPixmap;

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

    nWidth = spec.width;
    nHeight = spec.height;
    nChannels = spec.nchannels;
    cout << "Image Size: " << nWidth << "X" << nHeight << endl;

    cout << "channels: " << nChannels << endl;
    CreateOutputPixels(nWidth, nHeight, nChannels);
  
    int nScanlineSize = nWidth * nChannels * sizeof(float);
    if(!in -> read_image(TypeDesc::FLOAT, (char*)pfPixmap+(nHeight-1)*nScanlineSize, AutoStride, -nScanlineSize))
    {
        cerr << "Could not read input image for " << infilename << ", error = " << geterror() << endl;
    }

    in -> close();  // close the file
    delete in;    // free ImageInput
  }
}


/*
save image to file
*/
void writeimage(string outfilename)
{
  if(outfilename == "")
  {
    cout << "type the name of the imagefile you want to store:";
    cin >> outfilename;
  }
  // create the subclass instance of ImageOutput which can write the right kind of file format
  ImageOutput *out = ImageOutput::create(outfilename);
  if (!out)
  {
    cerr << "Could not create output image for " << outfilename << ", error = " << geterror() << endl;
  }
  else
  {
    // open and prepare the image file
    ImageSpec spec (nWidth, nHeight, nChannels, TypeDesc::FLOAT);
    out -> open(outfilename, spec);
    // write the entire image
    int nScanlineSize = nWidth * nChannels * sizeof(float);
    out -> write_image(TypeDesc::FLOAT, (char*)pfPixmap+(nHeight-1)*nScanlineSize, AutoStride, -nScanlineSize);
    cout << "Write the image pixmap to image file " << outfilename << endl;
    // close the file and free the ImageOutput I created
    out -> close();

    delete out;
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

/*
display image on screen
*/
void display()
{
   if(pfPixmap == NULL)
   {
	cout << "no image to present"<<endl;
	return;
   }

  glClearColor(1, 1, 1, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  int nWindowWidth=glutGet(GLUT_WINDOW_WIDTH);
  int nWindowHeight = glutGet(GLUT_WINDOW_HEIGHT);
  // display the pfPixmap
  float fRatio = 1;
  GetimageRatio(nWidth, nHeight, nWindowWidth, nWindowHeight, fRatio);
  glRasterPos2i(0.5*nWindowWidth-0.5*nWidth*fRatio, 0.5*nWindowHeight-0.5*nHeight*fRatio);
  glPixelZoom(fRatio, fRatio);
  // glDrawPixels writes a block of pixels to the framebuffer.
  switch (nChannels)
  {
    case 1:
      glDrawPixels(nWidth, nHeight, GL_LUMINANCE, GL_FLOAT, pfPixmap);
      break;
    case 3:
      glDrawPixels(nWidth, nHeight, GL_RGB, GL_FLOAT, pfPixmap);
      break;
    case 4:
      glDrawPixels(nWidth, nHeight, GL_RGBA, GL_FLOAT, pfPixmap);
      break;
    default:
      return;
  }
  glEnd();
  glFlush();
}






/*
Reshape Callback Routine: sets up the viewport and drawing coordinates
*/
void handleReshape(int w, int h)
{
  
  glViewport(0, 0, w, h);
	glLoadIdentity();
	gluOrtho2D(0, w, 0, h);// sets up a 2D orthographic viewing region
}



char **getIter(char** begin, char** end, const std::string& option)
{
    return find(begin, end, option);
}

/*
command line option parser
  MODE_GARBOR - filter the image via Gabor filter: filt <input_image_name> <output_image_name>(optional) -g theta sigma period
  MODE_FILTERFILE - filter the image via filter from file: filt <filter_name> <input_image_name>  <output_image_name>(optional)
*/
void getCmdOption(int argc, char **argv, string &inputImage, string &filter, string &outImage, double &theta, double &sigma, double &T, int &mode)
{
  if (argc < 3)
  {
    cout << "Help: " << endl;
    cout << "Gabor Filter: " << endl;
    cout << "[Usage] filt <input_image_name> <output_image_name>(optional) -g theta sigma period" << endl;
    cout << "Filter File: " << endl;
    cout << "[Usage] filt <input_image_name> <filter_name> <output_image_name>(optional)" << endl;
    exit(0);
  }
  char **iter = getIter(argv, argv + argc, "-g");
  // gabor mode
  if (iter != argv + argc)
  {
    mode = MODE_GARBOR; // mode 1: gabor filter mode
    cout << "Gabor Filter" << endl;
    if (argc >= 6)
    {
      if (++iter != argv + argc)
      {
        theta = atof(iter[0]);
        sigma = atof(iter[1]);
        T = atof(iter[2]);
        inputImage = argv[1];
        cout << "theta = " << theta << " sigma = " << sigma << " period = " << T << endl;
        if (argc == 7)  {outImage = argv[2];}
      }
      else  {cout << "Gabor Filter: " << endl << "[Usage] filt <input_image_name> -g theta sigma period" << endl; exit(0);}
    }
    else  {cout << "Gabor Filter: " << endl << "[Usage] filt <input_image_name> -g theta sigma period" << endl; exit(0);}
  }
  // normal mode
  else
  {
    mode = MODE_FILTFILE;
    cout << "Filter From File" << endl;
    if (argc >= 3)
    {
      inputImage = argv[2];
      filter = argv[1];
      cout << "Input Image: " << inputImage << endl;
      cout << "Filter File: " << filter << endl;
      if (argc == 4)  {outImage = argv[3];  cout << "Output Image: " << outImage << endl;}
    }
    else
    {
      cout << "Filter File: " << endl << "[Usage] filt <input_image_name> <filter_name> <output_image_name>(optional)" << endl;
      exit(0);
    }
  }
}

void handleKey(unsigned char key, int x, int y)
{
  switch(key)
  {
    case 'r':
    case 'R':
        readimage(strInputImgName);
        display();
        break;
    case 'w':
    case 'W':
      writeimage(strOutputImgName);
      break;
    case 'c':
    case 'C':
        cConvolution.filterImage(pfPixmap, nWidth, nHeight, nChannels);
	      display();
        break;
    // quit the program
    case 'q':		// q - quit
    case 'Q':
    case 27:		// esc - quit
      ReleasePixmap();
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
  string filter;    // filter file name
  
  int mode = 0; // mode = 1: gabor filter, mode = 2: filter from file
  double theta;   // gabor filter parameter: theta
  double sigma;   // gabor filter parameter: sigma
  double T;       // gabor filter parameter: period
  // command line parser
  getCmdOption(argc, argv, strInputImgName, filter, strOutputImgName, theta, sigma, T, mode);

  // read input image
  readimage(strInputImgName);
  // filter image
    if (mode == MODE_GARBOR)
    {
        cConvolution.getGaborFilter(theta, sigma, T);

    }
    else if (mode == MODE_FILTFILE)
    {
        cConvolution.readfilter(filter);
    }

 

  // display input image and output image in seperated windows
  // start up the glut utilities
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
  glutInitWindowSize(nWidth, nHeight);

  glutCreateWindow("Output Image");
  // set up the callback routines to be called when glutMainLoop() detects an event
  glutDisplayFunc(display);	  // display callback
 
  glutReshapeFunc(handleReshape); // window resize callback
  glutKeyboardFunc(handleKey);	  // keyboard callback

  glutMainLoop();

  // release memory
//  delete [] inputpixmap;
  ReleasePixmap();
  

  return 0;
}

