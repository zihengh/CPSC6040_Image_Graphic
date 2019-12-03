#include "MattingFunc.h"

MattingFunc::MattingFunc()
{
    data = NULL;
}

void MattingFunc::SetImageInfo(int nWidth, int nHeight, int nChannels, unsigned char* puchData)
{
    p_nWidth = nWidth;
    p_nHeight = nHeight;
    p_nChannels = nChannels;
    data = puchData;
    p_nStep = p_nWidth * p_nChannels;
}


double MattingFunc::pixelDistance(PixelPoint s, PixelPoint d)
{
    return sqrt(double((s.x - d.x) * (s.x - d.x) + (s.y - d.y) * (s.y - d.y)));
}

double MattingFunc::colorDistance2(Scalar cs1, Scalar cs2)
{
    return (cs1.uchV1 - cs2.uchV1) * (cs1.uchV1 - cs2.uchV1) +
    (cs1.uchV2 - cs2.uchV2) * (cs1.uchV2 - cs2.uchV2) +
    (cs1.uchV3 - cs2.uchV3) * (cs1.uchV3 - cs2.uchV3);
}



double MattingFunc::comalpha(Scalar c, Scalar f, Scalar b)
{
    double alpha = ((c.uchV1 - b.uchV1) * (f.uchV1 - b.uchV1) +
                    (c.uchV2 - b.uchV2) * (f.uchV2 - b.uchV2) +
                    (c.uchV3 - b.uchV3) * (f.uchV3 - b.uchV3))
    / ((f.uchV1 - b.uchV1) * (f.uchV1 - b.uchV1) +
       (f.uchV2 - b.uchV2) * (f.uchV2 - b.uchV2) +
       (f.uchV3 - b.uchV3) * (f.uchV3 - b.uchV3) + 0.0000001);
    return min(1.0, max(0.0, alpha));
}

double MattingFunc::chromaticDistortion(int i, int j, Scalar f, Scalar b)
{
    Scalar c = LOAD_RGB_SCALAR(data, i*p_nStep + j*p_nChannels);
    
    double alpha = comalpha(c, f, b);
    
    double result = sqrt((c.uchV1 - alpha * f.uchV1 - (1 - alpha) * b.uchV1) * (c.uchV1 - alpha * f.uchV1 - (1 - alpha) * b.uchV1) +
                         (c.uchV2 - alpha * f.uchV2 - (1 - alpha) * b.uchV2) * (c.uchV2 - alpha * f.uchV2 - (1 - alpha) * b.uchV2) +
                         (c.uchV3 - alpha * f.uchV3 - (1 - alpha) * b.uchV3) * (c.uchV3 - alpha * f.uchV3 - (1 - alpha) * b.uchV3));
    return result / 255.0;
}

double MattingFunc::neighborhoodAffinity(int i, int j, Scalar f, Scalar b)
{
    int i1 = max(0, i - 1);
    int i2 = min(i + 1, p_nHeight - 1);
    int j1 = max(0, j - 1);
    int j2 = min(j + 1, p_nWidth - 1);
    
    double  result = 0;
    
    for (int k = i1; k <= i2; ++k)
    {
        for (int l = j1; l <= j2; ++l)
        {
            double distortion = chromaticDistortion(k, l, f, b);
            result += distortion * distortion;
        }
    }
    
    return result;
}

double MattingFunc::energyOfPath(int i1, int j1, int i2, int j2)
{
    double ci = i2 - i1;    //path distance in x direction
    double cj = j2 - j1;    //path distance in y direction
    double z  = sqrt(ci * ci + cj * cj);    //path length
    
    double ei = ci / (z + 0.0000001);
    double ej = cj / (z + 0.0000001);
    
    double stepinc = min(1 / (abs(ei) + 1e-10), 1 / (abs(ej) + 1e-10));
    double result = 0;
    
    Scalar pre = LOAD_RGB_SCALAR(data, i1*p_nStep + j1*p_nChannels);
    
    int ti = i1;
    int tj = j1;
    
    for (double t = 1; ;t += stepinc)
    {
        double inci = ei * t;       //accumulation length in x direction
        double incj = ej * t;       //accumulation length in y direction
        int i = int(i1 + inci + 0.5);
        int j = int(j1 + incj + 0.5);
        
        double z = 1;
        
        Scalar cur = LOAD_RGB_SCALAR(data, i*p_nStep + j*p_nChannels);
        
        if (ti - i > 0 && tj - j == 0)
        {
            z = ej;
        }
        else if(ti - i == 0 && tj - j > 0)
        {
            z = ei;
        }
        
        result += ((cur.uchV1 - pre.uchV1) * (cur.uchV1 - pre.uchV1) +
                   (cur.uchV2 - pre.uchV2) * (cur.uchV2 - pre.uchV2) +
                   (cur.uchV3 - pre.uchV3) * (cur.uchV3 - pre.uchV3)) * z;
        pre = cur;
        
        ti = i;
        tj = j;
        if(abs(ci) <= abs(inci) || abs(cj) <= abs(incj))
            break;
        
        
    }
    
    return result;
}

double MattingFunc::probabilityOfForeground(PixelPoint p, vector<PixelPoint>& f, vector<PixelPoint>& b)
{
    double fmin = 1e10;
    vector<PixelPoint>::iterator it;
    for (it = f.begin(); it != f.end(); ++it) {
        double fp = energyOfPath(p.x, p.y, it->x, it->y);
        if (fp < fmin) {
            fmin = fp;
        }
    }
    
    double bmin = 1e10;
    for (it = b.begin(); it != b.end(); ++it) {
        double bp = energyOfPath(p.x, p.y, it->x, it->y);
        if (bp < bmin) {
            bmin = bp;
        }
    }
    return bmin / (fmin + bmin + 1e-10);
}

double MattingFunc::aP(int i, int j, double pf, Scalar f, Scalar b)
{
    Scalar c = LOAD_RGB_SCALAR(data, i*p_nStep + j*p_nChannels);
    double alpha = comalpha(c, f, b);
    
    return pf + (1 - 2 * pf) * alpha;
}

double MattingFunc::gP(PixelPoint p, PixelPoint fp, PixelPoint bp, double distance, double probability)
{
    Scalar f = LOAD_RGB_SCALAR(data, fp.x*p_nStep + fp.y*p_nChannels);
    Scalar b = LOAD_RGB_SCALAR(data, bp.x*p_nStep + bp.y*p_nChannels);
    
    double tn = pow(neighborhoodAffinity(p.x, p.y, f, b), EN);
    double ta = pow(aP(p.x, p.y, probability, f, b), EA);
    double tf = distance;
    double tb = pow(pixelDistance(p, bp), EB);
    
    return tn * ta * tf * tb;
}

double MattingFunc::sigma2(PixelPoint p)
{
    int xi = p.x;
    int yj = p.y;
    
    Scalar pc = LOAD_RGB_SCALAR(data, xi*p_nStep + yj*p_nChannels);
    
    int i1 = max(0, xi - 2);
    int i2 = min(xi + 2, p_nHeight - 1);
    int j1 = max(0, yj - 2);
    int j2 = min(yj + 2, p_nWidth - 1);
    
    double result = 0;
    int    num    = 0;
    
    for (int i = i1; i <= i2; ++i)
    {
        for (int j = j1; j <= j2; ++j)
        {
            Scalar temp = LOAD_RGB_SCALAR(data, i*p_nStep + j*p_nChannels);
            result += colorDistance2(pc, temp);
            ++num;
        }
    }
    
    return result / (num + 1e-10);
    
}


