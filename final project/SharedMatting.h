#ifndef SHAREDMSTTING_H
#define SHAREDMSTTING_H

#include <iostream>
# include <OpenImageIO/imageio.h>
#include <cmath>
#include <vector>
#include <string>
#include "MattStructDef.h"
#include "MattingFunc.h"

OIIO_NAMESPACE_USING
using namespace std;




class SharedMatting
{
public:
    SharedMatting();
    virtual ~SharedMatting();
    void loadTrimapFile(string filename);
    void loadTrimap(unsigned char* puchTemp, int nWidth, int nHeight, int nChannels);
    void readimage(string infilename, int &nWidth, int &nHeight, int &nChannels);
    void solveAlpha();
    void writeimage(string outfilename);
    unsigned char* GetOriginalPixel();
protected:
    void expandKnown();
    void gathering();
    void refineSample();
    void localSmooth();
    void sample(vector<vector<PixelPoint> > &F, vector<vector<PixelPoint> > &B);
    void getMatte();
        
private:
    unsigned char* p_puchInPixmap;  // input image pixel map
    unsigned char* p_puchTempPixmap;    //trimap image pixel map
    unsigned char* p_puchOutPixmap;  // output image pixel map
    
    vector<PixelPoint> p_vUnknownSet;
    vector<struct Tuple> p_vTuples;
    vector<struct Ftuple> p_vFtuples;
    
    int p_nHeight;
    int p_nWidth;
    int p_nChannels;
    int ** p_ppUnknownIndex;//information about Unknown area；
    int ** p_ppTriData; //information about trimap
    int ** p_ppAlpha;   // information about result alpha value
    unsigned char* data;
    int p_nStep;
    MattingFunc p_cMattingFunc;
    
};

#endif

