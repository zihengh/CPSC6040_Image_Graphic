#ifndef SHAREDMSTTING_STRUCT_DEFINE_H
#define SHAREDMSTTING_STRUCT_DEFINE_H

#define  KI      10    //  the 
#define  KC      5.0   //  D_COLOR,  for expansion of known region
#define  KG      4     //  for sample gathering, each unknown p gathers at most kG forground and background samples
#define  EN      3
#define  EA      2
#define  EB      4


#define  IS_BACKGROUND(x)  (x == 0)
#define  IS_FOREGROUND(x)  (x == 255)
#define  IS_KNOWN(x)       (IS_BACKGROUND(x) || IS_FOREGROUND(x))
#define  IS_UNKNOWN(x)     (!IS_KNOWN(x))
#define  LOAD_RGB_SCALAR(data, pos)    Scalar(data[pos], data[pos+1], data[pos+2])

//struct to record the position of pixel
struct PixelPoint
{
    int x;      //pixel row number
    int y;      //pixel col number
    PixelPoint()
    {
        x = 0;
        y = 0;
    }

    PixelPoint(int nx, int ny)
    {
        x = nx;
        y = ny;
    }
};

struct labelPoint
{
    int x;
    int y;
    int label;
};

//color value of a pixel
struct Scalar
{
    unsigned char uchV1;
    unsigned char uchV2;
    unsigned char uchV3;
    Scalar(unsigned char V1, unsigned char V2, unsigned char V3)
    {
        uchV1 = V1;
        uchV2 = V2;
        uchV3 = V3;
    }
    Scalar()
    {
        uchV1 = 0;
        uchV2 = 0;
        uchV3 = 0;
    }

};

struct Tuple
{
    Scalar f;
    Scalar b;
    double   sigmaf;
    double   sigmab;
    
    int flag;
    Tuple()
    {

        sigmaf = 0;
        sigmab = 0;
        flag = 0;
    }
};

struct Ftuple
{
    Scalar f;
    Scalar b;
    double   alphar;
    double   confidence;
};

#endif