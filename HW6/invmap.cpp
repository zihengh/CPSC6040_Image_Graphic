#include "invmap.h"
# include <iostream>

using namespace std;

InvMap::InvMap()
{

}

void InvMap::SetImageInfo(int invmode, int nRepairMode, int nWidthIn, int nHeightIn, int nWidthOut, int nHeightOut)
{
    p_nRepairMode = nRepairMode;
    p_nInvMode = invmode;
    p_nWidthIn = nWidthIn;
    p_nHeightIn = nHeightIn;
    p_nWidthOut = nWidthOut;
    p_nHeightOut = nHeightOut;
}

void InvMap::InverseMap(float x, float y, float &u, float &v)
{
    switch(p_nInvMode)
    {
        case MODE_INVERSE_1:
            inv_map(x, y, u, v);
            break;
        case MODE_INVERSE_2:
            inv_map2(x, y, u, v);
            break;
        default:
            return;
    }
}

void InvMap::inv_map(float x, float y, float &u, float &v)
{
  
  x /= p_nWidthOut;		// normalize (x, y) to (0...1, 0...1)
  y /= p_nHeightOut;

  u = x/2;
  v = y/2; 
  
  u *= p_nWidthIn;			// scale normalized (u, v) to pixel coords
  v *= p_nHeightIn;
}

void InvMap::inv_map2(float x, float y, float &u, float &v)
{
  
  x /= p_nWidthOut;		// normalize (x, y) to (0...1, 0...1)
  y /= p_nHeightOut;

  u = 0.5 * (x * x * x * x + sqrt(sqrt(y)));
  v = 0.5 * (sqrt(sqrt(x)) + y * y * y * y);
  
  u *= p_nWidthIn;			// scale normalized (u, v) to pixel coords
  v *= p_nHeightIn;
}

/*
scale factor calculator
  scale_factor > 1: minification
  scale_factor < 1: magnification
*/
void InvMap::scale_factor(double x, double y, double &scale_factor_x, double &scale_factor_y)
{
  // calculate four corners of one output pixel value in the input image
  // in order to calculate scale factor in each direction (x direction and y direction)
  float u0, v0, u1, v1, u2, v2, u3, v3;
  InverseMap(x - 0.5, y - 0.5, u0, v0);
  InverseMap(x + 0.5, y - 0.5, u1, v1);
  InverseMap(x - 0.5, y + 0.5, u2, v2);
  InverseMap(x + 0.5, y + 0.5, u3, v3);
  
  scale_factor_x = ((u1 - u0) + (u3 - u2)) / 2;
  scale_factor_y = ((v2 - v0) + (v3 - v1)) / 2;
}

unsigned char InvMap::bilinear_interpolation(double u, double v, int row_out, int col_out, int channel, unsigned char* puchPixmap)
{
  // calculate the positions of four points to do the bilinear interpolation
  double u0, v0, u1, v1, u2, v2, u3, v3, s, t;
  u0 = (u >= (floor(u) + 0.5)) ? (floor(u) + 0.5) : (floor(u) - 0.5);
  v0 = (v >= (floor(v) + 0.5)) ? (floor(v) + 0.5) : (floor(v) - 0.5);
  s = u - u0;
  t = v - v0;

  // boundary area
  if ((v >= p_nHeightIn - 0.5) || (v <= 0.5)) 
  {
    t = 0;
    v0 = (v >= p_nHeightIn - 0.5) ? (p_nHeightIn - 0.5) : (0.5);
  }
  if ((u >= p_nWidthIn - 0.5) || (u <= 0.5))
  {
    s = 0;
    u0 = (u >= p_nWidthIn - 0.5) ? (p_nWidthIn - 0.5) : (0.5);
  }

  u1 = u0 + 1;
  v1 = v0;
  u2 = u0;
  v2 = v0 + 1;
  u3 = u0 + 1;
  v3 = v0 + 1;

  // get the color of four points
  double c0, c1, c2, c3;
  unsigned char c_out;
  c0 = puchPixmap[(int(floor(v0)) * p_nWidthIn + int(floor(u0))) * 4 + channel];
  c1 = puchPixmap[(int(floor(v1)) * p_nWidthIn + int(floor(u1))) * 4 + channel];
  c2 = puchPixmap[(int(floor(v2)) * p_nWidthIn + int(floor(u2))) * 4 + channel];
  c3 = puchPixmap[(int(floor(v3)) * p_nWidthIn + int(floor(u3))) * 4 + channel];
  c_out = (1 - s) * (1 - t) * c0 + s * (1 - t) * c1 + (1 - s) * t * c2 + s * t * c3;
  
  return c_out;
}

void InvMap::supersampling(int row_in, int col_in, unsigned char* puchReconstruct, unsigned char* puchInputPixmap)
{
  double weight_list[] = {1.0, 2.0, 1.0, 
                          2.0, 8.0, 2.0, 
                          1.0, 2.0, 1.0};
  // get the multisamples
  int index_list [9] = {0};
  int index = 0;
  for (int i = -1; i <= 1; ++i)
  {
    for (int j = -1; j <= 1; ++j)
    {
      if ((row_in + i) < p_nHeightIn && (row_in + i) >= 0 && (col_in + j) < p_nWidthIn && (col_in + j) >= 0)
      {
         index_list[index] = (row_in + i) *  p_nWidthIn + (col_in + j);
      }
      // exclude samples located input image's outside
      else index_list[index] = -1;
      index++;
    }
  }
  for (int channel = 0; channel < 3; channel++)
  {
    double weight_count = 0;
    double sum = 0;
    unsigned char c_out;
    for (int i = 0; i < 9; ++i)
    {
      if (index_list[i] != -1)
      {
        sum += weight_list[i] * puchInputPixmap[index_list[i] * 4 + channel];
        weight_count += weight_list[i];
      }
    }
    c_out = sum / weight_count;
    puchReconstruct[(row_in * p_nWidthIn + col_in) * 4 + channel] = c_out;
  }
  
}

void InvMap::ad_supersampling(int row_in, int col_in, unsigned char* puchReconstruct, unsigned char* puchInputPixmap)
{
  double weight_list[] = {1.0, 2.0, 1.0, 
                          2.0, 8.0, 2.0, 
                          1.0, 2.0, 1.0};
  int index_list [9] = {0};
  int index = 0;
  for (int i = -1; i <= 1; ++i)
  {
    for (int j = -1; j <= 1; ++j)
    {
      if ((row_in + i) < p_nHeightIn && (row_in + i) >= 0 && (col_in + j) < p_nWidthIn && (col_in + j) >= 0)
        {index_list[index] = (row_in + i) * p_nWidthIn + (col_in + j);}
      // exclude samples located input image's outside
      else {index_list[index] = -1;}
      index++;
    }
  }
  
  for (int channel = 0; channel < 4; channel++)
  {
    // calculate samples average value
    double sample_sum = 0;
    double sample_count = 0;
    for (int i = 0; i < 9; i++)
    {
      if (index_list[i] != -1)
      {
        sample_sum += puchInputPixmap[index_list[i] * 4 + channel];
        sample_count++;
      }
    }
    double sample_avg = sample_sum / sample_count;
    // exclude extreme pixel which have a larger difference from average value than predefined threshold
    for (int i = 0; i < 9; i++)
    {
      if (index_list[i] != -1)
      {
        double diff = abs(puchInputPixmap[index_list[i] * 4 + channel] - sample_avg);
        if (diff > SAMPLING_TH) {index_list[i] = -1;}
      }
    }
  }

  for (int channel = 0; channel < 4; channel++)
  {
    double weight_count = 0;
    double sum = 0;
    unsigned char c_out;
    for (int i = 0; i < 9; ++i)
    {
      if (index_list[i] != -1)
      {
        sum += weight_list[i] * puchInputPixmap[index_list[i] * 4 + channel];
        weight_count += weight_list[i];
      }
    }
    c_out = sum / weight_count;
    puchReconstruct[(row_in * p_nWidthIn + col_in) * 4 + channel] = c_out;
  }
 
}