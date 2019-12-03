/**
  Matting.cpp
  Created by Zihengh on 18/11/20.
  Copyright © 2018 Zihengh. All rights reserved.
**/

# include <OpenImageIO/imageio.h>
#include <iostream>
#include "SharedMatting.h"
#include "alphamask.h"
//#include "sharedmatt.h"

# ifdef __APPLE__
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#   include <GLUT/glut.h>
# else
#   include <GL/glut.h>
# endif

using namespace std;
OIIO_NAMESPACE_USING
//static unsigned char* puchInputImage = NULL;
static unsigned char* puchAlphamask = NULL;
static int nWidth;
static int nHeight;
static int nChannels;
Alphamask cAlphamask;
SharedMatting cSharedMatting;
//SharedMatt cSharedMatt;
static bool bSolving = false;

bool loadTrimap()
{
  unsigned char uchTemp[nWidth*nHeight*3];
  
  memset(uchTemp, 0, nWidth*nHeight*3);
  if(!cAlphamask.GetTrimapData(uchTemp, nWidth, nHeight))
  {
    return false;
  }
  cSharedMatting.loadTrimap(uchTemp, nWidth, nHeight, 3);
  return true;
}


void handleKey(unsigned char key, int x, int y)
{
  if(bSolving == true)
  {
    return;
  }
  switch(key)
  {
    // write the current window to an image file
    case '1':
        cAlphamask.SetAdjustMode(MODE_H);
        break;
    case '2':
        cAlphamask.SetAdjustMode(MODE_S);
        break;
    case '3':
        cAlphamask.SetAdjustMode(MODE_V);
        break;
    case 'w':
    case 'W':
      cAlphamask.writeimage("");
      break;
    case 's':
    case 'S':
      bSolving = true;
      if(!loadTrimap())
      {
        bSolving = false;
        return;
      }
      cSharedMatting.solveAlpha();
      cSharedMatting.writeimage("");
      bSolving = false;
      break;
    case 'k':
    case 'K':
        cAlphamask.SetPixelPos(true);
        cout << "Known Area Settled" << endl;
        break;
    case 'u':
    case 'U':
        cAlphamask.SetPixelPos(false);
        cout << "Unknown Area Settled" << endl;
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

  if(puchAlphamask == NULL)
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
  glDrawPixels(nWidth, nHeight, GL_RGBA, GL_UNSIGNED_BYTE, puchAlphamask);
  glDisable(GL_BLEND);
  glEnd();
  glFlush();
}

void handleSpecialKey(int nKey, int x, int y)
{
    switch(nKey)
  {
    case GLUT_KEY_LEFT:
        cAlphamask.AdjustHSVRange(false, false);
        break;
    case GLUT_KEY_RIGHT:
        cAlphamask.AdjustHSVRange(false, true);
        break;
    case GLUT_KEY_UP:
        cAlphamask.AdjustHSVRange(true, true);
        break;
    case GLUT_KEY_DOWN:
        cAlphamask.AdjustHSVRange(true, false);
        break;
    default:		// not a valid key -- just ignore it
      return;
  }
  display();
}

char **getIter(char** begin, char** end, const std::string& option) 
{
  return find(begin, end, option);
}

int main(int argc, char * argv[]) 
{
    if(argc != 3)
    {
      return(1);
    }
    cSharedMatting.readimage(argv[1], nWidth, nHeight, nChannels);
//    cSharedMatt.readimage(argv[1], nWidth, nHeight, nChannels);
    char **iter = getIter(argv, argv + argc, "-t");
    if(iter != argv + argc)
    {
      string strTrimap;
      cout << "Please type the trimap you want to load: " ;
      cin >> strTrimap;
      cSharedMatting.loadTrimapFile(strTrimap);
      cSharedMatting.solveAlpha();
      cSharedMatting.writeimage("");
//      cSharedMatt.loadTrimapFile(strTrimap);
//      cSharedMatt.solveAlpha();
//      cSharedMatt.writeimage("");
    }
    else 
    {
      iter = getIter(argv, argv + argc, "-a");
      if (iter != argv + argc)
      {
        cAlphamask.SetImageInfo(cSharedMatting.GetOriginalPixel(), nWidth, nHeight, nChannels);
        puchAlphamask = cAlphamask.GetImagePixel();
        cAlphamask.alphamask();
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
        glutInitWindowSize(nWidth, nHeight);
        glutCreateWindow("Original Image");

        glutDisplayFunc(display);	  // display callback
        glutKeyboardFunc(handleKey);	  // keyboard callback
        glutReshapeFunc(handleReShapeWindow); // window resize callback
        glutSpecialFunc(handleSpecialKey);

        glutMainLoop();
      }

    }
    
    
    return 0;
}
