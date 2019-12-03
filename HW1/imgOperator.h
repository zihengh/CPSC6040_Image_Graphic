#ifndef IMGOPERATOR_H
#define IMGOPERATOR_H

#include <OpenImageIO/imageio.h>
#include <iostream>

#ifdef __APPLE__
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

using namespace std;
OIIO_NAMESPACE_USING

/*different channels to display*/
enum
{
    _DISPLAY_ORIGINAL = 0,  //display the original image
    _DISPLAY_REDCHANNEL,    //display red channel of image
    _DISPLAY_GREENCHANNEL,  //display green channel of image
    _DISPLAY_BLUECHANNEL,   //display blue channel of image
};

/*struct to store image pixel to display*/
typedef struct _RGB_PIXEL_STRUCT
{
    unsigned char uchRed;           //the value of red channel
    unsigned char uchGreen;         //the value of green channel
    unsigned char uchBlue;          //the value of blue channel
    unsigned char uchAlpha;         //the value of alpha channel
    _RGB_PIXEL_STRUCT()
    {
        uchRed=0;
        uchGreen=0;
        uchBlue=0;
        uchAlpha=255;               //the alpha channel is valued as 255 when the struct is created
    }
}RGB_PIXEL_STRUCT;

class imgOperator
{
    public:
        imgOperator();
        virtual ~imgOperator();
        static void LoadPixels(unsigned char* puchSrcPixel, _RGB_PIXEL_STRUCT* psrtDestRGBPixel,  int nWidth, int nHeight, int nChannel);
        static void GetImageRatio(int nImgWidth, int nImgHeight, int nWindowWidth, int nWindowHeight, float &fWRatio, float &fHRatio);
        static void ReOrganizePixelChannel(unsigned char uchDisplayMode, unsigned char* puchPixel, _RGB_PIXEL_STRUCT* psrtRGBPixel, int nWidth, int nHeight, int nChannels);
        static void InverseImgColorDisplayed(_RGB_PIXEL_STRUCT* psrtRGBPixel, int nWidth, int nHeight);
    protected:

    private:

        static void Load_RGBA_Pixels(unsigned char* puchSrcPixel, _RGB_PIXEL_STRUCT* psrtDestRGBPixel, int nWidth, int nHeight);
        static void Load_RGB_Pixels(unsigned char* puchSrcPixel, _RGB_PIXEL_STRUCT* psrtDestRGBPixel, int nWidth, int nHeight);
        static void Load_Single_Channel(unsigned char* puchSrcPixel, _RGB_PIXEL_STRUCT* psrtDestRGBPixel, int nWidth, int nHeight);
};

#endif // IMGOPERATOR_H
