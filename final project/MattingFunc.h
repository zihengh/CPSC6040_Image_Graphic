#ifndef MATTINGFUNC_H
#define MATTINGFUNC_H

#include <iostream>
#include <cmath>
#include <vector>
#include <string>
#include "MattStructDef.h"

using namespace std;



/*程序中认定cv::Point中 x为行，y为列，可能错误，但对程序结果没有影响*/
class MattingFunc
{
public:
    MattingFunc();
    void SetImageInfo(int nWidth, int nHeight, int nChannels, unsigned char* puchData);
     double chromaticDistortion(int i, int j, Scalar f, Scalar b);
     double neighborhoodAffinity(int i, int j, Scalar f, Scalar b);
    
    
     double aP(int i, int j, double pf, Scalar f, Scalar b);
     double gP(PixelPoint p, PixelPoint fp, PixelPoint bp, double distance, double probability);
    
     double energyOfPath(int i1, int j1, int i2, int j2);
     double probabilityOfForeground(PixelPoint p, vector<PixelPoint>& f, vector<PixelPoint>& b);
     double pixelDistance(PixelPoint s, PixelPoint d);
     double colorDistance2(Scalar cs1, Scalar cs2);
     double sigma2(PixelPoint p);
     double comalpha(Scalar c, Scalar f, Scalar b);
private:
    int p_nWidth;
    int p_nHeight;
    int p_nChannels;
    int p_nStep;
    unsigned char* data;
};

#endif

