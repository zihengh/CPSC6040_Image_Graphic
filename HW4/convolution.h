#ifndef _CLASS_CONVOLUTION_H
#define _CLASS_CONVOLUTION_H

# include <stdlib.h>
# include <iostream>
# include <fstream>
# include <string>
# include <algorithm>
# include <math.h>
# include <cmath>
# include <iomanip>

using namespace std;

class Convolution
{
    public:
        Convolution();
        virtual ~Convolution();
        void filterImage(float* pfPixmap, int nWidth, int nHeight, int nChannels);
        void readfilter(string filterfile);
        void getGaborFilter(double theta, double sigma, double T);
    private:

        int reflectBorder(int index, int total);
        void convolve(double **in, double **out, int nWidth, int nHeight);
    private:
        double **kernel;   // 2D-array to store convolutional kernel
        int kernel_size;   // kernel size
        
};


#endif