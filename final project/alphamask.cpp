
#include "alphamask.h"
# include <cstdlib>
# include <iostream>
# include <fstream>
# include <string>
# include <OpenImageIO/imageio.h>


using namespace std;
OIIO_NAMESPACE_USING

Alphamask::Alphamask()
{
    p_puchInputImage = NULL;
    p_puchOutputImage=NULL;
    puchKnown=NULL;
    puchUnknown=NULL;
    p_bKnownSet=false;
    p_bUnknownSet=false;
}

Alphamask::~Alphamask()
{
    if(p_puchOutputImage!=NULL)
    {
        delete []p_puchOutputImage;
        p_puchOutputImage=NULL;
    }
    if(puchKnown!=NULL)
    {
        delete []puchKnown;
        puchKnown=NULL;
    }
    if(puchUnknown!=NULL)
    {
        delete []puchUnknown;
        puchUnknown=NULL;
    }

}

unsigned char* Alphamask::GetImagePixel()
{
    return p_puchOutputImage;
}

void Alphamask::SetImageInfo(unsigned char* puchpixel, int nWidth, int nHeight, int nChannels)
{
    p_puchInputImage = puchpixel;
    if(p_puchInputImage == NULL)
    {
        exit(1);
    }
    p_nWidth = nWidth;
    p_nHeight = nHeight;
    p_nChannels = nChannels;
    p_puchOutputImage = new unsigned char[nWidth*nHeight*4];
    puchKnown = new unsigned char[nWidth*nHeight];
    puchUnknown = new unsigned char[nWidth*nHeight];
}

void Alphamask::SetAdjustMode(unsigned char uchMode)
{
    uchValMode = uchMode;
}

/*
convert RGB color image to HSV color image
  input RGB color primary values: r, g, and b on scale 0 - 255
  Output HSV colors: h on scale 0-360, s and v on scale 0-1
*/
void Alphamask::RGBtoHSV(int r, int g, int b, double &h, double &s, double &v)
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


void Alphamask::get_thresholds()
{
  fstream thresholdsFile("thresholds.txt");
thresholdsFile >> db_hl >> db_hh >> db_sl >> db_sh >> db_vl >> db_vh >> db_ghl >> db_ghh >> db_spill_s >> db_spill_k;
}



bool Alphamask::GetTrimapData(unsigned char* puchTrimap, int nWidth, int nHegiht)
{

    if(puchTrimap==NULL)
    {
        return false;
    }
    if(!p_bKnownSet || !p_bUnknownSet)
    {
        cout << "please finish setting the known and unknown area";
        return false;
    }
    if(puchKnown == NULL || puchUnknown == NULL)
    {
        return false;
    }
    for(int i=0; i<nHegiht; i++)
    {
        for(int j=0; j<nWidth; j++)
        {
            if(puchKnown[i*nWidth+j]==255)
            {
                puchTrimap[i*nWidth*3+j*3]=255;
                puchTrimap[i*nWidth*3+j*3+1]=255;
                puchTrimap[i*nWidth*3+j*3+2]=255;
            }
            else if(puchUnknown[i*nWidth+j]==255)  
            {
                puchTrimap[i*nWidth*3+j*3]=128;
                puchTrimap[i*nWidth*3+j*3+1]=128;
                puchTrimap[i*nWidth*3+j*3+2]=128;
            }
        }
    }
   
    return true;
}


/*
write out the trimap
*/
void Alphamask::writeimage(string outfilename)
{
    if(!p_bKnownSet || !p_bUnknownSet)
    {
        cout << "please finish setting the known and unknown area";
        return ;
    }
    if(puchKnown == NULL || puchUnknown ==NULL)
    {
        return ;
    }
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
    unsigned char* puchTrimap = new unsigned char[p_nWidth*p_nHeight*3];
    memset(puchTrimap, 0, p_nWidth*p_nHeight*3*sizeof(unsigned char));
    
    for(int i=0; i<p_nHeight; i++)
    {
        for(int j=0; j<p_nWidth; j++)
        {
            if(puchKnown[i*p_nWidth+j]==255)
            {
                puchTrimap[i*p_nWidth*3+j*3]=255;
                puchTrimap[i*p_nWidth*3+j*3+1]=255;
                puchTrimap[i*p_nWidth*3+j*3+2]=255;
            }
            else if(puchUnknown[i*p_nWidth+j]==255)  
            {
                puchTrimap[i*p_nWidth*3+j*3]=128;
                puchTrimap[i*p_nWidth*3+j*3+1]=128;
                puchTrimap[i*p_nWidth*3+j*3+2]=128;
            }
        }
    }
    // open and prepare the image file
    ImageSpec spec (p_nWidth, p_nHeight, 3, TypeDesc::UINT8);
    out -> open(outfilename, spec);
    // write the entire image
    int nScanlineSize = p_nWidth * 3 * sizeof(unsigned char);
    out -> write_image(TypeDesc::UINT8, puchTrimap + (p_nHeight-1)*nScanlineSize, AutoStride, -nScanlineSize);
    cout <<  outfilename << " has been stored" << endl;
    // close the file and free the imageOutput i created
    out -> close();

    delete out;
    delete []puchTrimap;
  }
}

bool Alphamask::IsInRange(double dbVal, double dbL, double dbH)
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

void Alphamask::alphamask()
{
  char chszThreshold[512] = {0};
  // get thresholds
  sprintf(chszThreshold, "h:%f~%f\ns:%f~%f\nv:%f~%f", db_hl, db_hh, db_sl, db_sh, db_vl, db_vh);
  cout << chszThreshold << endl;
  sprintf(chszThreshold, "k1:%f  k2:%f", db_spill_s, db_spill_k);
  cout << chszThreshold << endl;
  // convert rgb color to hsv color
  for (int nCol = 0; nCol < p_nWidth; nCol++)
  {
    for (int nRow = 0; nRow < p_nHeight; nRow++)
    {
      int r, g, b;
      double h, s, v;
 
      r = p_puchInputImage[(nRow * p_nWidth + nCol) * 3];
      g = p_puchInputImage[(nRow * p_nWidth + nCol) * 3 + 1];
      b = p_puchInputImage[(nRow * p_nWidth + nCol) * 3 + 2];


      RGBtoHSV(r, g, b, h, s, v);
    


      p_puchOutputImage[(nRow * p_nWidth + nCol) * 4] = r;
      p_puchOutputImage[(nRow * p_nWidth + nCol) * 4 + 1] = g;
      p_puchOutputImage[(nRow * p_nWidth + nCol) * 4 + 2] = b;
      // generate alpha channel mask
      

      if(!IsInRange(h, db_hl, db_hh))
      {
        p_puchOutputImage[(nRow * p_nWidth + nCol) * 4 + 3] = 255 ;
        continue;
      }
      if(!IsInRange(s, db_sl, db_sh))
      {

        p_puchOutputImage[(nRow * p_nWidth + nCol) * 4 + 3] = 255 ;
        continue;
      }
      if(!IsInRange(v, db_vl, db_vh))
      {
        p_puchOutputImage[(nRow * p_nWidth + nCol) * 4 + 3] = 255 ;
        continue;
      }

      p_puchOutputImage[(nRow * p_nWidth + nCol) * 4 + 3] = 0;
    }
  }
}


void Alphamask::AdjustHSVRange(bool bUpperBound, bool bIncrease)
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
}

void Alphamask::SetPixelPos(bool bKnown)
{
    if(puchKnown == NULL || puchUnknown ==NULL)
    {
        return ;
    }
    if(bKnown)
    {
         memset(puchKnown, 0, p_nWidth*p_nHeight*sizeof(unsigned char));
    }
    else memset(puchUnknown, 0, p_nWidth*p_nHeight*sizeof(unsigned char));
   
    
    for(int i=0; i<p_nHeight; i++)
    {
        for(int j=0; j<p_nWidth; j++)
        {
            if(p_puchOutputImage[i*p_nWidth*4+j*4+3]==255)
            {
                if(bKnown)
                {
                    puchKnown[i*p_nWidth+j]=255;
                }
                else puchUnknown[i*p_nWidth+j]=255;
            } 

        }
    }
    if(bKnown)
    {
        p_bKnownSet = true;
    }
    else p_bUnknownSet = true;
}






