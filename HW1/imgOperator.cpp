#include "imgOperator.h"
//#include<string>

imgOperator::imgOperator()
{
    //ctor

}

imgOperator::~imgOperator()
{
    //dtor
}


/*To Inverse the color of the image pixels on display
[in]_RGB_PIXEL_STRUCT* psrtRGBPixel:the pixel memory you want to inverse
[in]int nWidth: picture width
[in]int nHeight:picture height
*/
void imgOperator::InverseImgColorDisplayed(_RGB_PIXEL_STRUCT* psrtRGBPixel, int nWidth, int nHeight)
{
    int nPos = 0;
  for(int nPixelRow=nHeight-1; nPixelRow>=0; nPixelRow--)
  {
    for(int nPixelLine=0; nPixelLine<nWidth; nPixelLine++)
    {
        nPos = nPixelRow*nWidth+nPixelLine;
        psrtRGBPixel[nPos].uchRed = 255-psrtRGBPixel[nPos].uchRed;
        psrtRGBPixel[nPos].uchGreen = 255-psrtRGBPixel[nPos].uchGreen;
        psrtRGBPixel[nPos].uchBlue = 255-psrtRGBPixel[nPos].uchBlue;
    }
  }
}

/*load the pixels read from image file to the pixel memory which is intended to be displayed in window
[in]unsigned char* puchSrcPixel:the pixel read from file
[in]_RGB_PIXEL_STRUCT* psrtDestRGBPixel:the pixel memory you want to inverse
[in]int nWidth: picture width
[in]int nHeight:picture height
[in]int nChannel:picture channel number
*/
void imgOperator::LoadPixels(unsigned char* puchSrcPixel, _RGB_PIXEL_STRUCT* psrtDestRGBPixel, int nWidth, int nHeight, int nChannel)
{
  switch(nChannel)
  {
  case 1:
    Load_Single_Channel(puchSrcPixel, psrtDestRGBPixel, nWidth, nHeight);
    break;
  case 2:
  case 3:
    Load_RGB_Pixels(puchSrcPixel, psrtDestRGBPixel,nWidth, nHeight);
    break;
  case 4:
    Load_RGBA_Pixels(puchSrcPixel, psrtDestRGBPixel,nWidth, nHeight);
    break;
  }

}

/*load a single channel image*/
void imgOperator::Load_Single_Channel(unsigned char* puchSrcPixel, _RGB_PIXEL_STRUCT* psrtDestRGBPixel, int nWidth, int nHeight)
{
  unsigned char* pPixelPos=puchSrcPixel;
  for(int nPixelRow=nHeight-1; nPixelRow>=0; nPixelRow--)
  {
    for(int nPixelLine=0; nPixelLine<nWidth; nPixelLine++)
    {
        psrtDestRGBPixel[nPixelRow*nWidth+nPixelLine].uchRed = *pPixelPos;
        psrtDestRGBPixel[nPixelRow*nWidth+nPixelLine].uchGreen = *pPixelPos;
        psrtDestRGBPixel[nPixelRow*nWidth+nPixelLine].uchBlue = *pPixelPos;
        pPixelPos++;
    }
  }
}

/*load a three channel image*/
void imgOperator::Load_RGB_Pixels(unsigned char* puchSrcPixel, _RGB_PIXEL_STRUCT* psrtDestRGBPixel, int nWidth, int nHeight)
{
  unsigned char* pPixelPos=puchSrcPixel;
  for(int nPixelRow=nHeight-1; nPixelRow>=0; nPixelRow--)
  {
    for(int nPixelLine=0; nPixelLine<nWidth; nPixelLine++)
    {
        psrtDestRGBPixel[nPixelRow*nWidth+nPixelLine].uchRed = *pPixelPos;
        pPixelPos++;
        psrtDestRGBPixel[nPixelRow*nWidth+nPixelLine].uchGreen = *pPixelPos;
        pPixelPos++;
        psrtDestRGBPixel[nPixelRow*nWidth+nPixelLine].uchBlue = *pPixelPos;
        pPixelPos++;
    }
  }
}

/*load a RGBA  image*/
void imgOperator::Load_RGBA_Pixels(unsigned char* puchSrcPixel, _RGB_PIXEL_STRUCT* psrtDestRGBPixel, int nWidth, int nHeight)
{
  unsigned char* pPixelPos=puchSrcPixel;
  for(int nPixelRow=nHeight-1; nPixelRow>=0; nPixelRow--)
  {
    for(int nPixelLine=0; nPixelLine<nWidth; nPixelLine++)
    {
        psrtDestRGBPixel[nPixelRow*nWidth+nPixelLine].uchRed = *pPixelPos;
        pPixelPos++;
        psrtDestRGBPixel[nPixelRow*nWidth+nPixelLine].uchGreen = *pPixelPos;
        pPixelPos++;
        psrtDestRGBPixel[nPixelRow*nWidth+nPixelLine].uchBlue = *pPixelPos;
        pPixelPos++;
        psrtDestRGBPixel[nPixelRow*nWidth+nPixelLine].uchAlpha = *pPixelPos;
        pPixelPos++;
    }
  }
}

/*the iamge pixels stored in memory have to be reorganized if to display a certain channel*/
void imgOperator::ReOrganizePixelChannel(unsigned char uchDisplayMode, unsigned char* puchPixel, _RGB_PIXEL_STRUCT* psrtRGBPixel, int nWidth, int nHeight, int nChannels)
{
    unsigned char* puchPos = puchPixel;
    if(uchDisplayMode != _DISPLAY_ORIGINAL)
    {
        if(nChannels==1)
        {
            for(int nPixelNum=0; nPixelNum<nWidth*nHeight; nPixelNum++)
            {
                *(puchPos+uchDisplayMode-1) = *((unsigned char*)&(psrtRGBPixel[nPixelNum])+uchDisplayMode-1);
                *(puchPos+3)=255;
                puchPos=puchPos+4;
            }
            return;
        }
        for(int nPixelNum=0; nPixelNum<nWidth*nHeight; nPixelNum++)
        {
            *puchPos = *((unsigned char*)&(psrtRGBPixel[nPixelNum])+uchDisplayMode-1);
            puchPos++;
        }
        return;
    }
    // if the user want to switch back to original form, the pixels have to be changed back to it's original form
    for(int nPixelRow=nHeight -1; nPixelRow>=0; nPixelRow--)
    {
        for(int nPixelLine=0; nPixelLine<nWidth; nPixelLine++)
        {
            *puchPos = psrtRGBPixel[nPixelRow*nWidth+nPixelLine].uchRed;
            puchPos++;

            if(nChannels>1)
            {
                *puchPos = psrtRGBPixel[nPixelRow*nWidth+nPixelLine].uchGreen;
                puchPos++;
                *puchPos = psrtRGBPixel[nPixelRow*nWidth+nPixelLine].uchBlue;
                puchPos++;
            }
            if(nChannels>3)
            {
                *puchPos = psrtRGBPixel[nPixelRow*nWidth+nPixelLine].uchAlpha;
                puchPos++;
            }
        }
    }

}

/*Get the ratio of the displayed image size to its original size
[in]int nImgWidth: the original image width
[in]int nImgHeight: the original image height
[in]int nWindowWidth: the current window width
[in]int nWindowHeight: the current window height
[out]float &fWRatio: the ratio of the iamge width
[out]float &fHRatio: the ratio of iamge height */
void imgOperator::GetImageRatio(int nImgWidth, int nImgHeight, int nWindowWidth, int nWindowHeight, float &fWRatio, float &fHRatio)
{
    if(nImgWidth<nWindowWidth && nImgHeight < nWindowHeight)
    {
        return ;
    }
    if(nImgWidth * nWindowHeight > nImgHeight * nWindowWidth)
    {
        fWRatio = fHRatio = ((float)nWindowWidth) / ((float)nImgWidth) ;
        return ;
    }
    if(nImgWidth * nWindowHeight <= nImgHeight * nWindowWidth)
    {
        fWRatio = fHRatio =  ((float)nWindowHeight)/ ((float)nImgHeight);
        return ;
    }
}




