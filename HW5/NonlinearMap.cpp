#include "NonlinearMap.h"
# include <iostream>

using namespace std;

# define max(x, y) (x > y ? x : y)
# define min(x, y) (x < y ? x : y)

NonlinearMap::NonlinearMap()
{
    p_dbCx = 0;
    p_dbCy = 0;
    p_dbStrength = 0;
    p_dbTx=0;
    p_dbTy=0;
    p_dbAx=0;
    p_dbAy=0;
}

void NonlinearMap::SetTwirl(double dbCx, double dbCy, double dbStrength)
{
    p_dbCx=dbCx;
    p_dbCy=dbCy;
    p_dbStrength = dbStrength;
}

void NonlinearMap::SetRipple(double dbTx, double dbTy, double dbAx, double dbAy)
{
    p_dbTx = dbTx;
    p_dbTy = dbTy;
    p_dbAx = dbAx;
    p_dbAy = dbAy;
}

void NonlinearMap::InverseTwirl(unsigned char* puchOutPixel, unsigned char* puchInPixel, int nWidthOut, int nHeightOut, int nWidthIn, int nHeightIn)
{   
  if(puchOutPixel == NULL)
  {
      return ;
  }
  if(puchInPixel == NULL)
  {
      return ;
  }
  double theta = 0;
  double r = 0;
  double md = min(nWidthIn, nHeightIn);
  for (int row_out = 0; row_out < nHeightOut; row_out++)  // output image row
  {
    for (int col_out = 0; col_out < nWidthOut; col_out++)  // output image col
    {
      r = pow(pow(col_out-p_dbCx,2) + pow(row_out-p_dbCy,2), 0.5);
      if(row_out>p_dbCy)
      {
        theta = acos((col_out - p_dbCx)/r);
      }  
      else theta = -acos((col_out - p_dbCx)/r);
      
      int row_in, col_in;
      row_in = p_dbCy + r*sin(theta + p_dbStrength*(r-md)/md);
      col_in = p_dbCx + r*cos(theta + p_dbStrength*(r-md)/md);
      
      if (row_in < nHeightIn && row_in >= 0 && col_in < nWidthIn && col_in >= 0)
      {
        for (int k = 0; k < 4; k++)
        {
            puchOutPixel[(row_out * nWidthOut + col_out) * 4 + k] = puchInPixel[(row_in * nWidthIn + col_in) * 4 + k];
        }
      }
    }
  }
  cout << "Twirl inverse complete." << endl;
  cout << "Press Q or q to quit." << endl;
}

void NonlinearMap::InverseRipple(unsigned char* puchOutPixel, unsigned char* puchInPixel, int nWidthOut, int nHeightOut, int nWidthIn, int nHeightIn)
{
  if(puchOutPixel == NULL)
  {
      return ;
  }
  if(puchInPixel == NULL)
  {
      return ;
  }

  for (int row_out = 0; row_out < nHeightOut; row_out++)  // output image row
  {
    for (int col_out = 0; col_out < nWidthOut; col_out++)  // output image col
    {
      
      int row_in, col_in;
      row_in = row_out + p_dbAy*sin(2*M_PI*col_out/p_dbTy);
      col_in = col_out + p_dbAx*sin(2*M_PI*row_out/p_dbTx);
      
      if (row_in < nHeightIn && row_in >= 0 && col_in < nWidthIn && col_in >= 0)
      {
        for (int k = 0; k < 4; k++)
        {
            puchOutPixel[(row_out * nWidthOut + col_out) * 4 + k] = puchInPixel[(row_in * nWidthIn + col_in) * 4 + k];
        }
      }
    }
  }
  cout << "Ripple inverse complete." << endl;
  cout << "Press Q or q to quit." << endl;
}