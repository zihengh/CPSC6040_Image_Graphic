#ifndef ALPHAMASK_H
#define ALPHAMASK_H

#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include "MattStructDef.h"

using namespace std;

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


/*程序中认定cv::Point中 x为行，y为列，可能错误，但对程序结果没有影响*/
class Alphamask
{
public:
    Alphamask();
    ~Alphamask();
    unsigned char* GetImagePixel();
    bool GetTrimapData(unsigned char* puchTrimap, int nWidth, int nHegiht);
    void SetImageInfo(unsigned char* puchpixel, int nWidth, int nHeight, int nChannels);
    void SetAdjustMode(unsigned char uchMode);
    void RGBtoHSV(int r, int g, int b, double &h, double &s, double &v);
    void get_thresholds();
    void writeimage(string outfilename);
    bool IsInRange(double dbVal, double dbL, double dbH);
    void alphamask();
    void GetimageRatio(int nImgWidth, int nImgHeight, int nWindowWidth, int nWindowHeight, float &fRatio);
    void AdjustHSVRange(bool bUpperBound, bool bIncrease);
    void SetPixelPos(bool bKnown);
private:
    int p_nWidth;
    int p_nHeight;
    int p_nChannels;
    int p_nStep;
    unsigned char* p_puchInputImage;
    unsigned char* p_puchOutputImage;
    unsigned char *puchKnown = NULL;
    unsigned char *puchUnknown = NULL;
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
    bool p_bKnownSet;
    bool p_bUnknownSet;
};

#endif

