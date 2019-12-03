#include "convolution.h"

using namespace std;

Convolution::Convolution()
{
    kernel = NULL;
    kernel_size = 0;
}

Convolution::~Convolution()
{
    for (int i = 0; i < kernel_size; i++)  
    {
        delete [] kernel[i];
    }
    delete [] kernel;
}

/******************************************************************
This function create the kernel for gabor filt according to given auguments
[in]double theta:angular orientation
[in]double sigm:standard deviation of Gaussian curve
[in]double T :period of cosine
*****************************************************************/
void Convolution::getGaborFilter(double theta, double sigma, double T)
{
//  double angle = theta;
  int kernel_center = 2 * sigma;
  double angle = theta * M_PI / 180;
  kernel_size = 2 * sigma + 1;
//  cout << cos(angle) << endl;
//  cout << cos(45) << endl;
  kernel = new double *[kernel_size];
  for (int i = 0; i < kernel_size; i++)  {kernel[i] = new double [kernel_size];}

  double positive_sum = 0;
  double negative_sum = 0;
  for (int row = 0; row < kernel_size; row++)
  {
    for (int col = 0; col < kernel_size; col++)
    {
      double x, y, xx, yy;
      // calculate the distance to kernel center
      x = (col > 0) ? ((col + 0.5) - kernel_center) : ((col - 0.5) - kernel_center);
      y = (row > 0) ? ((row + 0.5) - kernel_center) : ((row - 0.5) - kernel_center);
//      x = row-(kernel_size-1)/2;
//      y = col-(kernel_size-1)/2;

      xx = x * cos(angle) + y * sin(angle);
      yy = -x * sin(angle) + y * cos(angle);
      kernel[row][col] = exp(-(pow(xx, 2.0) + pow(yy, 2.0)) / (2 * pow(sigma, 2.0))) * cos(2 * M_PI * xx / T);

      if (kernel[row][col] > 0) {positive_sum += kernel[row][col];}
      else  {negative_sum += (-kernel[row][col]);}

    }
    cout << endl;
  }

  double scale = (positive_sum > negative_sum) ? positive_sum : negative_sum;

  for (int row = 0; row < kernel_size; row++)
  {
    for (int col = 0; col < kernel_size; col++)
    {
      kernel[row][col] = kernel[row][col] / scale;
    }
  }
}

/******************************************************************
This function read the filter file
[in]string filterfile: the directory and name of the filterfile
*****************************************************************/
void Convolution::readfilter(string filterfile)
{
  fstream filterFile(filterfile.c_str());
  // get kernel size

  filterFile >> kernel_size;
  // allocate memory for kernel 2D-array
  kernel = new double *[kernel_size];
  for (int i = 0; i < kernel_size; i++)
  {
    kernel[i] = new double [kernel_size];
  }
  // get kernel values
  double positive_sum = 0;
  double negative_sum = 0;
  cout << "Kernel Size: " << kernel_size << endl;
  cout << "Kernel " << filterfile << ": " << endl;
  for (int row = 0; row < kernel_size; row++)
  {
    for (int col = 0; col < kernel_size; col++)
    {
      filterFile >> kernel[row][col];
      cout << kernel[row][col] << " ";
      if (kernel[row][col] > 0) {positive_sum += kernel[row][col];}
      else  {negative_sum += (-kernel[row][col]);}
    }
    cout << endl;
  }
  // kernel normalization
  double scale = (positive_sum > negative_sum) ? positive_sum : negative_sum;
  cout << "Scale Factor: " << scale << endl;
  for (int row = 0; row < kernel_size; row++)
  {
    for (int col = 0; col < kernel_size; col++)
    {
      kernel[row][col] = kernel[row][col] / scale;
    }
  }
}

/******************************************************************
This function deal with the boundary problem
[in]int index: the current row/col index
[in]int total: the total row/col number
[out]int :the new index of row/col
*****************************************************************/
int Convolution::reflectBorder(int index, int total)
{
  int index_new;
  index_new = index;
  if (index < 0)  {index_new = -index;}
  if (index >= total) {index_new = 2 * (total - 1) - index;}

  return index_new;
}

/******************************************************************
This function do convolution to pixels
[in]double **in: the original pixel value
[out]double **out: the result pixel value
[in]int nWidth: the image width
[in]int nHeight: the image height
*****************************************************************/
void Convolution::convolve(double **in, double **out, int nWidth, int nHeight)
{
  int n = (kernel_size - 1) / 2;
  for (int row = -n; row < nHeight - n; row++)
  {
    for (int col = -n; col < nWidth - n; col++)
    {
      double sum = 0;
      // inside boundary
      if (row <= (nHeight - kernel_size) and row >= 0 and col <= (nWidth - kernel_size) and col >= 0)
      {
        for (int i = 0; i < kernel_size; i++)
        {
          for (int j = 0; j < kernel_size; j++)
          {
            sum += kernel[kernel_size - 1 - i][kernel_size - 1 - j] * in[row + i][col + j];
          }
        }
      }
      // boundary conditions
      // reflect image at borders to add points for rows and cols outside the boundaries
      else
      {
        for (int i = 0; i < kernel_size; i++)
        {
          for (int j = 0; j < kernel_size; j++)
          {
            sum += kernel[kernel_size - 1 - i][kernel_size - 1 - j] * in[reflectBorder(row + i, nHeight)][reflectBorder(col + j, nWidth)];
          }
        }
      }
      out[row + n][col + n] = sum;
    }
  }
}

/******************************************************************
This function do filts image
[in]float* pfPixmap: the original pixels of the entire image stored in memory
[in]int nWidth: the image width
[in]int nHeight: the image height
[in]int nChannels: the image channel number
*****************************************************************/
void Convolution::filterImage(float* pfPixmap, int nWidth, int nHeight, int nChannels)
{
  int m = (nWidth > nHeight) ? nWidth : nHeight;
  double **channel_value = new double *[m];
  double **out_value = new double *[m];
  for (int i = 0; i < m; i++)
  {
    channel_value[i] = new double [m];
    out_value[i] = new double [m];
  }
  
  for (int channel = 0; channel < nChannels; channel++)
  {
    if(channel == 3)
    {
        continue;
    }
    for (int row = 0; row < nHeight; row++)
    {
      for (int col = 0; col < nWidth; col++)
      {
        // store the channel value on scale 0-1
        channel_value[row][col] = pfPixmap[(row * nWidth + col) * nChannels + channel];
      }
    }
    convolve(channel_value, out_value, nWidth, nHeight);
    for (int row = 0; row < nHeight; row++)
    {
      for (int col = 0; col < nWidth; col++)
      {
        // scale the output value to 0-255: 255 times the absolute value
        pfPixmap[(row * nWidth + col) * nChannels + channel] = abs(out_value[row][col]);
      }
    }
  }

  // release memory
  for (int i = 0; i < m; i++)  
  {
      delete [] channel_value[i]; 
      delete [] out_value[i];
    }
  delete [] channel_value;
  delete [] out_value;
}