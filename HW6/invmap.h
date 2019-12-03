#ifndef _INVMAP_HEADER_H_
#define _INVMAP_HEADER_H_

#include <cstdio>
#include <cmath>

# define SAMPLING_TH 65 // predefined adaptive supersampling threshold, 

enum 
{
    MODE_INVERSE_1 = 1,
    MODE_INVERSE_2,
    MODE_INVERSE_INVALID = 255,
};

enum
{
    MODE_REPAIR_BILINEAR = 1,
    MODE_REPAIR_SUPERSAMPLING,
    MODE_REPAIR_AD_SUPERSAMPLE,
    MODE_REPAIR_AUTO,
    MODE_REPAIR_INVALID = 255,
};

class InvMap
{
public:
  InvMap();
  void InverseMap(float x, float y, float &u, float &v);
  void SetImageInfo(int invmode, int nRepairMode, int nWidthIn, int nHeightIn, int nWidthOut, int nHeightOut);
  void scale_factor(double x, double y, double &scale_factor_x, double &scale_factor_y);
  unsigned char bilinear_interpolation(double u, double v, int row_out, int col_out, int channel, unsigned char* puchPixmap);
  void supersampling(int row_in, int col_in, unsigned char* puchReconstruct,  unsigned char* puchInputPixmap);
  void ad_supersampling(int row_in, int col_in, unsigned char* puchReconstruct,  unsigned char* puchInputPixmap);
private:
  void inv_map(float x, float y, float &u, float &v);
  void inv_map2(float x, float y, float &u, float &v);
private:
  int p_nInvMode;
  int p_nRepairMode;
  int p_nWidthIn;
  int p_nHeightIn;
  int p_nWidthOut;
  int p_nHeightOut;
};

#endif