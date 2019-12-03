#ifndef _NONLINEARMAP_HEADER_H_
#define _NONLINEARMAP_HEADER_H_

#include <cstdio>
#include <cmath>



class NonlinearMap
{

  
public:
  NonlinearMap();
  void SetTwirl(double dbCx, double dbCy, double dbStrength);
  void SetRipple(double dbTx, double dbTy, double dbAx, double dbAy);
  void InverseTwirl(unsigned char* puchOutPixel, unsigned char* puchInPixel, int nWidthOut, int nHeightOut, int nWidthIn, int nHeightIn);
  void InverseRipple(unsigned char* puchOutPixel, unsigned char* puchInPixel, int nWidthOut, int nHeightOut, int nWidthIn, int nHeightIn);
private:
  double p_dbCx;
  double p_dbCy;
  double p_dbStrength;
  double p_dbTx;
  double p_dbTy;
  double p_dbAx;
  double p_dbAy;
};

#endif