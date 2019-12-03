#include <OpenImageIO/imageio.h>
#include <iostream>
#include<vector>
#include<string>

#ifdef __APPLE__
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#include "imgOperator.h"

using namespace std;
OIIO_NAMESPACE_USING


unsigned char* puchPixel = NULL;            //
_RGB_PIXEL_STRUCT* psrtRGBPixel = NULL;

int nWidth = 0;
int nHeight = 0;
int nChannels = 0;
int nCurrentImgIdx = 0;
unsigned char uchDisplayMode=0;
vector<string> vImageFileName;

/* check if there are image pixels stored in memory */
bool IsImageOnScreen()
{
    if(psrtRGBPixel == NULL || puchPixel == NULL)
    {
        return false;
    }
    return true;
}

/* analyze command line
    will return false if the command doesn't start as imgview*/
bool ResolveCommand()
{
    string strCommand;
    cout << "Please enter new command (e.g imgview xxx.png)" << endl;
    vImageFileName.clear();
    getline(cin, strCommand);

    if(strCommand == "")
        return false;

    char* strs = new char[strCommand.length()+1];
    strcpy(strs, strCommand.c_str());

    //split the command line string by " "
    char *p = strtok(strs, " ");
    while(p)
    {
        string s = p;
        vImageFileName.push_back(s);
        p = strtok(NULL, " ");
    }
    delete []strs;

    if(vImageFileName[0].compare("exit")==0)
    {
        exit(0);
        return true;
    }
    if(vImageFileName[0].compare("imgview")==0)
    {
        return true;
    }
    cout << "ilegal command"<<endl;
    return false;
}

/*to change the image displayed on window
    return false when there is no former or next image*/
bool SwitchImgOnDisplay(bool bFormerImg)
{
    int nImgNum = vImageFileName.size();
    if(nImgNum<1)
    {
        return false;
    }
    if(bFormerImg)
    {
        if(nCurrentImgIdx-1<0)
        {
            return false;
        }
        nCurrentImgIdx = nCurrentImgIdx-1;
        return true;
    }

    if(nCurrentImgIdx>=nImgNum-2)
    {
        return false;
    }
    nCurrentImgIdx = nCurrentImgIdx + 1;
    return true;
}


/*to release all pixels stored in memory*/
void ReleasePixels()
{
    if(psrtRGBPixel!=NULL)
    {
        delete []psrtRGBPixel;
        psrtRGBPixel=NULL;
    }

    if(puchPixel==NULL)
    {
        return;
    }
    delete []puchPixel;
    puchPixel=NULL;
}

/*allocate memory for image pixels
    4*imagewidth*imageheight for pixels on display
    imagewidth*imageheight*channelnumber for the original file*/
void CreatePixelMem(int nWidth, int nHeight, int nChannels)
{
    ReleasePixels();
    puchPixel = new unsigned char[nWidth*nHeight*nChannels];
    psrtRGBPixel = new _RGB_PIXEL_STRUCT[nWidth*nHeight];
}

/* display image in window*/
void DisplayImage()
{
    int nWindowWidth=glutGet(GLUT_WINDOW_WIDTH);
    int nWindowHeight = glutGet(GLUT_WINDOW_HEIGHT);

    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(!IsImageOnScreen())
    {
        glEnd();
        glFlush();
        return ;
    }
//
    float fWRatio = 1;
    float fHRatio = 1;
    imgOperator::GetImageRatio(nWidth, nHeight, nWindowWidth, nWindowHeight, fWRatio, fHRatio);
    glRasterPos2i(0.5*nWindowWidth-0.5*nWidth*fWRatio, 0.5*nWindowHeight-0.5*nHeight*fHRatio);

    glPixelZoom(fWRatio, fHRatio);
    if(nChannels == 1 && uchDisplayMode!=_DISPLAY_ORIGINAL)
    {
        glDrawPixels(nWidth, nHeight, GL_RGBA,GL_UNSIGNED_BYTE,puchPixel);
        glEnd();
        glFlush();
        return;
    }
    switch(uchDisplayMode)
    {
        case _DISPLAY_ORIGINAL:
            glDrawPixels(nWidth, nHeight, GL_RGBA,GL_UNSIGNED_BYTE,psrtRGBPixel);
            break;
        case _DISPLAY_REDCHANNEL:
            glDrawPixels(nWidth, nHeight, GL_RED,GL_UNSIGNED_BYTE,puchPixel);
            break;
        case _DISPLAY_GREENCHANNEL:
            glDrawPixels(nWidth, nHeight, GL_GREEN,GL_UNSIGNED_BYTE,puchPixel);
            break;
        case _DISPLAY_BLUECHANNEL:
            glDrawPixels(nWidth, nHeight, GL_BLUE,GL_UNSIGNED_BYTE,puchPixel);
            break;
    }


    glEnd();

    glFlush();
}

/*display different channels of image*/
void SwitchChannelOnDisplay(unsigned char uchChannelOnDisplay)
{
    if(uchChannelOnDisplay==uchDisplayMode)
    {
        return;
    }
    uchDisplayMode = uchChannelOnDisplay;

    delete []puchPixel;
    puchPixel=NULL;
    if(uchChannelOnDisplay==_DISPLAY_ORIGINAL)
    {
        puchPixel = new unsigned char[nWidth*nHeight*nChannels];
    }
    else if(nChannels==1)
    {
        puchPixel = new unsigned char[nWidth*nHeight*4];
        memset(puchPixel, 0, nWidth*nHeight*4);
    }
    else puchPixel = new unsigned char[nWidth*nHeight];


    imgOperator::ReOrganizePixelChannel(uchChannelOnDisplay, puchPixel, psrtRGBPixel, nWidth, nHeight, nChannels);
    DisplayImage();
}

/*read single image file according to the commandline and the current file index*/
void ReadImage()
{
    if(vImageFileName.size()<2)
        return;
    ImageInput *in = ImageInput::open(vImageFileName[nCurrentImgIdx+1]);
    if(!in)
    {
        cerr << "Could not create Input image for " << vImageFileName[nCurrentImgIdx+1] << ", error = " << geterror() << endl;
        return;
    }
    const ImageSpec &spec = in->spec();
    nWidth = spec.width;
    nHeight = spec.height;
    nChannels = spec.nchannels;

    CreatePixelMem(nWidth, nHeight, nChannels);

    in->read_image(TypeDesc::UINT8, &puchPixel[0]);
    in->close();
    ImageInput::destroy(in);
    imgOperator::LoadPixels(puchPixel, psrtRGBPixel, nWidth, nHeight, nChannels);

    uchDisplayMode = _DISPLAY_ORIGINAL;
}

/* this function only used when user trys to store the red or green or blue channels of image in the window*/
void SaveScreenShot(ImageOutput *outfile, string outfilename)
{
  int w = glutGet(GLUT_WINDOW_WIDTH);
  int h = glutGet(GLUT_WINDOW_HEIGHT);
  unsigned char pixmap[4 * w * h] = {0};
  unsigned char pixmapflip[4 * w * h]= {0};

  glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixmap);
  unsigned char* pPixelPos=pixmapflip;
  for(int nPixelRow=h-1; nPixelRow>=0; nPixelRow--)
  {
    for(int nPixelLine=0; nPixelLine<w; nPixelLine++)
    {
        *pPixelPos = pixmap[4*nPixelRow*w+4*nPixelLine];
        pPixelPos++;
        *pPixelPos = pixmap[4*nPixelRow*w+4*nPixelLine+1] ;
        pPixelPos++;
        *pPixelPos= pixmap[4*nPixelRow*w+4*nPixelLine+2] ;
        pPixelPos++;
        *pPixelPos= pixmap[4*nPixelRow*w+4*nPixelLine+3] ;
        pPixelPos++;
    }
  }

  ImageSpec spec(w, h, 4, TypeDesc::UINT8);
  if(!outfile->open(outfilename, spec)){
    cerr << "Could not open " << outfilename << ", error = " << geterror() << endl;
    ImageOutput::destroy(outfile);
    return;
  }

  // write the image to the file. All channel values in the pixmap are taken to be
  // unsigned chars
  if(!outfile->write_image(TypeDesc::UINT8, pixmapflip)){
    cerr << "Could not write image to " << outfilename << ", error = " << geterror() << endl;
    ImageOutput::destroy(outfile);
    return;
  }
  else
    cout << outfilename << " is stored" << endl;

  // close the image file after the image is written
  if(!outfile->close()){
    cerr << "Could not close " << outfilename << ", error = " << geterror() << endl;
    ImageOutput::destroy(outfile);
    return;
  }

  // free up space associated with the oiio file handler
  delete outfile;
}

/*write the original form of the image currently displayed in the window*/
void WriteImage()
{
    if(!IsImageOnScreen())
    {
        cout << "there is currently no image to write ";
        return ;
    }
    string outfilename;

  // get a filename for the image. The file suffix should indicate the image file
  // type. For example: output.png to create a png image file named output
  cout << "enter output image filename: ";
  cin >> outfilename;
  ImageOutput *outfile = ImageOutput::create(outfilename);
  if(!outfile){
    cerr << "Could not create output image for " << outfilename << ", error = " << geterror() << endl;
    return;
  }
  if(uchDisplayMode != _DISPLAY_ORIGINAL)
  {
    SaveScreenShot(outfile, outfilename);
    return;
  }

  ImageSpec spec(nWidth, nHeight, nChannels, TypeDesc::UINT8);
  if(!outfile->open(outfilename, spec)){
    cerr << "Could not open " << outfilename << ", error = " << geterror() << endl;
    ImageOutput::destroy(outfile);
    return;
  }
   if(!outfile->write_image(TypeDesc::UINT8, puchPixel)){
    cerr << "Could not write image to " << outfilename << ", error = " << geterror() << endl;
    ImageOutput::destroy(outfile);
    return;
  }
  else cout << outfilename << " is stored" << endl;

  // close the image file after the image is written
  if(!outfile->close()){
    cerr << "Could not close " << outfilename << ", error = " << geterror() << endl;
    ImageOutput::destroy(outfile);
    return;
  }

  // free up space associated with the oiio file handler
  delete outfile;
}
/*
   Keyboard Callback Routine:
   This routine is called every time a key is pressed on the keyboard
*/
void handleKey(unsigned char key, int x, int y){

  switch(key)
  {
    case '1':
        if(!IsImageOnScreen())
            return;
        SwitchChannelOnDisplay(_DISPLAY_REDCHANNEL);
        break;
    case '2':
        if(!IsImageOnScreen())
            return;
        SwitchChannelOnDisplay(_DISPLAY_GREENCHANNEL);
        break;
    case '3':
        if(!IsImageOnScreen())
            return;
        SwitchChannelOnDisplay(_DISPLAY_BLUECHANNEL);
        break;
    case 'O':
    case 'o':
        SwitchChannelOnDisplay(_DISPLAY_ORIGINAL);
        break;
    case 'i':
    case 'I':
        if(!IsImageOnScreen())
            return;
        imgOperator::InverseImgColorDisplayed(psrtRGBPixel, nWidth, nHeight);
        uchDisplayMode = _DISPLAY_ORIGINAL;
        DisplayImage();
        break;
    case 'r':
    case 'R':
        if(ResolveCommand())
        {
            ReadImage();
        }
        glutReshapeWindow(nWidth, nHeight);
        glutPostRedisplay();
        DisplayImage();
        break;
    case 'w':
    case 'W':
        WriteImage();
        break;
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
        if(SwitchImgOnDisplay(true))
        {
            ReadImage();
            glutReshapeWindow(nWidth, nHeight);
            glutPostRedisplay();
            DisplayImage();
        }
        break;
    case GLUT_KEY_RIGHT:
        if(SwitchImgOnDisplay(false))
        {
            ReadImage();
            glutReshapeWindow(nWidth, nHeight);
            glutPostRedisplay();
            DisplayImage();
        }
        break;
    default:		// not a valid key -- just ignore it
      return;
  }
}

void handleReShapeWindow(int nW, int nH)
{
    glViewport(0, 0, nW, nH);

	// define the drawing coordinate system on the viewport
//	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, nW, 0, nH);// sets up a 2D orthographic viewing region
//	DisplayImage();
}

int main(int argc, char* argv[])
{
    vImageFileName.clear();
    for(int nFileIdx=0; nFileIdx<argc; nFileIdx++)
    {
        cout << argv[nFileIdx] << endl;
        vImageFileName.push_back(argv[nFileIdx]);
    }
    ReadImage();



    glutInit(&argc, argv);

  // create the graphics window, giving width, height, and title text
    glutInitDisplayMode( GLUT_RGBA | GLUT_SINGLE);
    glutInitWindowSize(nWidth, nHeight);
    glutCreateWindow("ImageView");

    glutPositionWindow(500, 500);


    glutDisplayFunc(DisplayImage);	  // display callback

    glutKeyboardFunc(handleKey);	  // keyboard callback
    glutSpecialFunc(handleSpecialKey);
    glutReshapeFunc(handleReShapeWindow); // window resize callback
//
    glutMainLoop();
    ReleasePixels();
    return 0;
}


