#include "SharedMatting.h"



# ifdef __APPLE__
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#   include <GLUT/glut.h>
# else
#   include <GL/glut.h>
# endif


SharedMatting::SharedMatting()
{
    p_vUnknownSet.clear();
    p_vTuples.clear();
    p_vFtuples.clear();

    p_puchInPixmap = NULL;
    p_puchTempPixmap = NULL;
    p_puchOutPixmap = NULL;
}

SharedMatting::~SharedMatting()
{

    p_vUnknownSet.clear();
    p_vTuples.clear();
    p_vFtuples.clear();
    
    for (int i = 0; i < p_nHeight; ++i)
    {
        delete[] p_ppTriData[i];
        delete[] p_ppUnknownIndex[i];
        delete[] p_ppAlpha[i];
    }
    delete[] p_ppTriData;
    delete[] p_ppUnknownIndex;
    delete[] p_ppAlpha;

    delete []p_puchInPixmap;
    delete []p_puchOutPixmap;
}



unsigned char* SharedMatting::GetOriginalPixel()
{
    return p_puchInPixmap;
}

/*
get the image puchPixmap
*/
void SharedMatting::readimage(string infilename, int &nWidth, int &nHeight, int &nChannels)
{
  ImageInput *in = ImageInput::open(infilename);
  if (!in)
  {
    cerr << "Cannot get the input image for " << infilename << ", error = " << geterror() << endl;
    return ;
  }
  // get the image size and channels information, allocate space for the image
    const ImageSpec &spec = in -> spec();
    nWidth = p_nWidth = spec.width;
    nHeight = p_nHeight = spec.height;
    nChannels = p_nChannels = spec.nchannels;
    cout << "width: " << p_nWidth << " height: " << p_nHeight << " channels: " << p_nChannels << endl;
    p_nStep = p_nWidth*p_nChannels*sizeof(unsigned char);
    p_puchInPixmap = new unsigned char [p_nWidth * p_nHeight * p_nChannels];
    int nScanlineSize = p_nWidth * p_nChannels * sizeof(unsigned char);
    if(!in -> read_image(TypeDesc::UINT8, p_puchInPixmap+(p_nHeight-1)*nScanlineSize, AutoStride, -nScanlineSize))
    {
        cerr << "Could not read input image for " << infilename << ", error = " << geterror() << endl;
    }
    in -> close();  // close the file
    ImageInput::destroy(in);
    data = p_puchInPixmap;
    cout << "image read successfully" << endl;

    p_ppUnknownIndex  = new int*[p_nHeight];
    p_ppTriData           = new int*[p_nHeight];
    p_ppAlpha         = new int*[p_nHeight];
    for(int i = 0; i < p_nHeight; ++i)
    {
        p_ppUnknownIndex[i] = new int[p_nWidth];
        p_ppTriData[i]          = new int[p_nWidth];
        p_ppAlpha[i]        = new int[p_nWidth];
    }
    p_cMattingFunc.SetImageInfo(p_nWidth, p_nHeight, p_nChannels, data);
}

/*
write out the result image
*/
void SharedMatting::writeimage(string outfilename)
{
  if(outfilename == "")
  {
    cout << "type the name of the imagefile you want to store:";
    cin >> outfilename;
  }
  // create the subclass instance of imageOutput which can write the right kind of file format
  ImageOutput *out = ImageOutput::create(outfilename);
  if (!out)
  {
    cerr << "Could not create output image for " << outfilename << ", error = " << geterror() << endl;
  }
  else
  {
    // open and prepare the image file
    ImageSpec spec (p_nWidth, p_nHeight, 4, TypeDesc::UINT8);
    out -> open(outfilename, spec);
    // write the entire image
    int nScanlineSize = p_nWidth * 4 * sizeof(unsigned char);
    out -> write_image(TypeDesc::UINT8, p_puchOutPixmap + (p_nHeight-1)*nScanlineSize, AutoStride, -nScanlineSize);
    cout <<  outfilename << " has been stored" << endl;
    // close the file and free the imageOutput i created
    out -> close();

    delete out;
  }
}

//  Trimap value: 0 - background, 255 - foreground,  others(128) - unknown.
void SharedMatting::loadTrimapFile(string filename)
{
  ImageInput *in = ImageInput::open(filename);
  if (!in)
  {
    cerr << "Cannot get the input image for " << filename << ", error = " << geterror() << endl;
    return ;
  }
  // get the image size and channels information, allocate space for the image
    const ImageSpec &spec = in -> spec();
    int nWidth, nHeight, nChannels, nStep;
    nWidth = spec.width;
    nHeight = spec.height;
    nChannels = spec.nchannels;
    cout << "width: " << nWidth << " height: " << nHeight << " channels: " << nChannels << endl;
    nStep = nWidth*nChannels*sizeof(unsigned char);
    p_puchTempPixmap = new unsigned char [nWidth * nHeight * nChannels];
    int nScanlineSize = nWidth * nChannels * sizeof(unsigned char);
    if(!in -> read_image(TypeDesc::UINT8, p_puchTempPixmap+(nHeight-1)*nScanlineSize, AutoStride, -nScanlineSize))
    {
        cerr << "Could not read input image for " << filename << ", error = " << geterror() << endl;
    }
    in -> close();  // close the file
    ImageInput::destroy(in);
    cout << "trimap read successfully" << endl;
    for (int i = 0; i < nHeight; ++i)
    {
        for (int j = 0; j < nWidth; ++j) 
        {
            p_ppTriData[i][j] = p_puchTempPixmap[i * nStep + j * nChannels];
        }
    }
    delete []p_puchTempPixmap;
    p_puchTempPixmap = NULL;
}

//  Trimap value: 0 - background, 255 - foreground,  others(128) - unknown.
void SharedMatting::loadTrimap(unsigned char* puchTemp, int nWidth, int nHeight, int nChannels)
{  
     cout << "width: " << nWidth << " height: " << nHeight << " channels: " << nChannels << endl;
    int nStep = nWidth * nChannels*sizeof(unsigned char);
    for (int i = 0; i < nHeight; ++i)
    {
        for (int j = 0; j < nWidth; ++j) 
        {
            p_ppTriData[i][j] = puchTemp[i * nStep + j * nChannels];
        }
    }
}

void SharedMatting::solveAlpha()
{
    //expandKnown()

    cout << "Expanding...";
    expandKnown();
    cout << "    over!!!" << endl;
    
    //gathering()
    cout << "Gathering...";
    gathering();
    cout << "    over!!!" << endl;
 
    
    cout << "Refining...";
    refineSample();
    cout << "    over!!!" << endl;
   
    
    
    cout << "LocalSmoothing..." ;
    localSmooth();
    cout << "    over!!!" << endl;

    getMatte();
}




void SharedMatting::expandKnown()
{
    vector<struct labelPoint> vp;
    int kc2 = KC * KC;

    for (int i = 0; i < p_nHeight; ++i) {
        for (int j = 0; j < p_nWidth; ++j) {
            if ( IS_UNKNOWN(p_ppTriData[i][j]) ) {
                int label = -1;
                bool bLabel = false;
                Scalar p = LOAD_RGB_SCALAR(data, i*p_nStep+j*p_nChannels);
                
                for (int k = 1; (k <= KI) && !bLabel; ++k) {
                    int k1 = max(0, i - k);
                    int k2 = min(i + k, p_nHeight - 1);
                    int l1 = max(0, j - k);
                    int l2 = min(j + k, p_nWidth - 1);
                    
                    for (int l = k1; (l <= k2) && !bLabel; ++l) {
                        double dis;
                        double gray;
                        
                        gray = p_ppTriData[l][l1];
                        if (IS_KNOWN(gray)) {
                            dis = p_cMattingFunc.pixelDistance(PixelPoint(i, j), PixelPoint(l, l1));
                            if (dis > KI) {
                                continue;
                            }
                            Scalar q = LOAD_RGB_SCALAR(data, l*p_nStep + l1*p_nChannels);
                            
                            double distanceColor = p_cMattingFunc.colorDistance2(p, q);
                            if (distanceColor <= kc2) {
                                bLabel = true;
                                label = gray;
                            }
                        }
                        if (bLabel) {
                            break;
                        }
                        
                        gray = p_ppTriData[l][l2];
                        if (IS_KNOWN(gray)) {
                            dis = p_cMattingFunc.pixelDistance(PixelPoint(i, j), PixelPoint(l, l2));
                            if (dis > KI) {
                                continue;
                            }
                            Scalar q = LOAD_RGB_SCALAR(data, l*p_nStep + l2*p_nChannels);
                            
                            double distanceColor = p_cMattingFunc.colorDistance2(p, q);
                            if (distanceColor <= kc2) {
                                bLabel = true;
                                label = gray;
                            }
                        }
                    }
                    
                    for (int l = l1; (l <= l2) && !bLabel; ++l) {
                        double dis;
                        double gray;
                        
                        gray = p_ppTriData[k1][l];
                        if (IS_KNOWN(gray)) {
                            dis = p_cMattingFunc.pixelDistance(PixelPoint(i, j), PixelPoint(k1, l));
                            if (dis > KI) {
                                continue;
                            }
                            
                            Scalar q = LOAD_RGB_SCALAR(data, k1*p_nStep+l*p_nChannels);
                            
                            double distanceColor = p_cMattingFunc.colorDistance2(p, q);
                            if (distanceColor <= kc2) {
                                bLabel = true;
                                label = gray;
                            }
                        }
                        gray = p_ppTriData[k2][l];
                        if (IS_KNOWN(gray)) {
                            dis = p_cMattingFunc.pixelDistance(PixelPoint(i, j), PixelPoint(k2, l));
                            if (dis > KI) {
                                continue;
                            }
                            Scalar q = LOAD_RGB_SCALAR(data, k2*p_nStep+l*p_nChannels);
                            
                            double distanceColor = p_cMattingFunc.colorDistance2(p, q);
                            if (distanceColor <= kc2) {
                                bLabel = true;
                                label = gray;
                            }
                        }
                    }
                }
                if (label != -1) {
                    struct labelPoint lp;
                    lp.x = i;
                    lp.y = j;
                    lp.label = label;
                    vp.push_back(lp);
                } else {
                    PixelPoint lp;
                    lp.x = i;
                    lp.y = j;
                    p_vUnknownSet.push_back(lp);
                }
            }
        }
    }
    
    vector<struct labelPoint>::iterator it;
    for (it = vp.begin(); it != vp.end(); ++it)
    {
        int ti = it->x;
        int tj = it->y;
        int label = it->label;
        p_ppTriData[ti][tj] = label;
    }
}


void SharedMatting::sample(std::vector<vector<PixelPoint> > &foregroundSamples, std::vector<vector<PixelPoint> > &backgroundSamples)
{
    int   a,b,i;
    int   x,y,p,q;
    int   w,h,gray;
    int   angle;
    double z,ex,ey,t,step;
    vector<PixelPoint>::iterator iter;
    
    a=360/KG;
    b=1.7f*a/9;
    foregroundSamples.clear();
    backgroundSamples.clear();
    w=p_nWidth;
    h=p_nHeight;
    for(iter=p_vUnknownSet.begin();iter!=p_vUnknownSet.end();++iter) {
        vector<PixelPoint> fPts,bPts;
        
        x=iter->x;
        y=iter->y;
        angle=(x+y)*b % a;
        for(i=0;i<KG;++i) {
            bool f1(false),f2(false);
            
            z=(angle+i*a)/180.0f*3.1415926f;
            ex=sin(z);
            ey=cos(z);
            step=min(1.0f/(abs(ex)+1e-10f),
                     1.0f/(abs(ey)+1e-10f));
            
            for(t=0;;t+=step) {
                p=(int)(x+ex*t+0.5f);
                q=(int)(y+ey*t+0.5f);
                if(p<0 || p>=h || q<0 || q>=w)
                    break;
                
                gray=p_ppTriData[p][q];
                if(!f1 &&  IS_BACKGROUND(gray)) {
                    PixelPoint pt = PixelPoint(p, q);
                    bPts.push_back(pt);
                    f1=true;
                } else {
                    if(!f2 && IS_FOREGROUND(gray)) {
                        PixelPoint pt = PixelPoint(p, q);
                        fPts.push_back(pt);
                        f2=true;
                    } else {
                        if(f1 && f2) break;
                    }
                }
            }
        }
        
        foregroundSamples.push_back(fPts);
        backgroundSamples.push_back(bPts);
    }
}

void SharedMatting::gathering()
{
    vector<PixelPoint> f;
    vector<PixelPoint> b;
    vector<PixelPoint>::iterator it1;
    vector<PixelPoint>::iterator it2;
    
    vector<vector<PixelPoint> > foregroundSamples, backgroundSamples;
    
    
    sample(foregroundSamples, backgroundSamples);
    
    int index = 0;
    int size = p_vUnknownSet.size();
    
    for (int m = 0; m < size; ++m)
    {
        int i = p_vUnknownSet[m].x;
        int j = p_vUnknownSet[m].y;
        
        double probability = p_cMattingFunc.probabilityOfForeground(PixelPoint(i, j), foregroundSamples[m], backgroundSamples[m]);
        double gmin = 1.0e10;
        
        PixelPoint tf;
        PixelPoint tb;
        
        bool flag = false;
        
        for (it1 = foregroundSamples[m].begin(); it1 != foregroundSamples[m].end(); ++it1)
        {
            double distance = p_cMattingFunc.pixelDistance(PixelPoint(i, j), *(it1));
            for (it2 = backgroundSamples[m].begin(); it2 < backgroundSamples[m].end(); ++it2)
            {
                
                double gp = p_cMattingFunc.gP(PixelPoint(i, j), *(it1), *(it2), distance, probability);
                if (gp < gmin)
                {
                    gmin = gp;
                    tf   = *(it1);
                    tb   = *(it2);
                    flag = true;
                }
            }
        }
        
        Tuple st;
        st.flag = -1;
        if (flag)
        {
            st.flag   = 1;
            st.f      = LOAD_RGB_SCALAR(data, tf.x*p_nStep + tf.y*p_nChannels);
            st.b      = LOAD_RGB_SCALAR(data, tb.x*p_nStep + tb.y*p_nChannels);
            st.sigmaf = p_cMattingFunc.sigma2(tf);
            st.sigmab = p_cMattingFunc.sigma2(tb);
        }
        
        p_vTuples.push_back(st);
        p_ppUnknownIndex[i][j] = index;
        ++index;
    }
}

void SharedMatting::refineSample()
{
    p_vFtuples.resize(p_nWidth * p_nHeight + 1);
    for (int i = 0; i < p_nHeight; ++i) {
        for (int j = 0; j < p_nWidth; ++j) {
            Scalar c = LOAD_RGB_SCALAR(data, i*p_nStep + j*p_nChannels);
            int indexf = i * p_nWidth + j;
            int gray = p_ppTriData[i][j];
            if ( IS_BACKGROUND(gray) ) {
                p_vFtuples[indexf].f = c;
                p_vFtuples[indexf].b = c;
                p_vFtuples[indexf].alphar = 0;
                p_vFtuples[indexf].confidence = 1;
                p_ppAlpha[i][j] = 0;
            } else if ( IS_FOREGROUND(gray) ) {
                p_vFtuples[indexf].f = c;
                p_vFtuples[indexf].b = c;
                p_vFtuples[indexf].alphar = 1;
                p_vFtuples[indexf].confidence = 1;
                p_ppAlpha[i][j] = 255;
            }
        }
    }
    vector<PixelPoint>::iterator it;
    for (it = p_vUnknownSet.begin(); it != p_vUnknownSet.end(); ++it)
    {
        int xi = it->x;
        int yj = it->y;
        int i1 = max(0, xi - 5);
        int i2 = min(xi + 5, p_nHeight - 1);
        int j1 = max(0, yj - 5);
        int j2 = min(yj + 5, p_nWidth - 1);
        
        double minvalue[3] = {1e10, 1e10, 1e10};
        PixelPoint * p = new PixelPoint[3];
        int num = 0;
        for (int k = i1; k <= i2; ++k)
        {
            for (int l = j1; l <= j2; ++l)
            {
                int temp = p_ppTriData[k][l];
                
                if (temp == 0 || temp == 255)
                {
                    continue;
                }
                
                int index = p_ppUnknownIndex[k][l];
                Tuple t   = p_vTuples[index];
                if (t.flag == -1)
                {
                    continue;
                }
                
                double m  = p_cMattingFunc.chromaticDistortion(xi, yj, t.f, t.b);
                
                if (m > minvalue[2])
                {
                    continue;
                }
                
                if (m < minvalue[0])
                {
                    minvalue[2] = minvalue[1];
                    p[2]   = p[1];
                    
                    minvalue[1] = minvalue[0];
                    p[1]   = p[0];
                    
                    minvalue[0] = m;
                    p[0].x = k;
                    p[0].y = l;
                    
                    ++num;
                    
                }
                else if (m < minvalue[1])
                {
                    minvalue[2] = minvalue[1];
                    p[2]   = p[1];
                    
                    minvalue[1] = m;
                    p[1].x = k;
                    p[1].y = l;
                    
                    ++num;
                }
                else if (m < minvalue[2])
                {
                    minvalue[2] = m;
                    p[2].x = k;
                    p[2].y = l;
                    
                    ++num;
                }
            }
        }
        
        num = min(num, 3);
        
        
        double fb = 0;
        double fg = 0;
        double fr = 0;
        double bb = 0;
        double bg = 0;
        double br = 0;
        double sf = 0;
        double sb = 0;
        
        for (int k = 0; k < num; ++k)
        {
            int i  = p_ppUnknownIndex[p[k].x][p[k].y];
            fb += p_vTuples[i].f.uchV1;
            fg += p_vTuples[i].f.uchV2;
            fr += p_vTuples[i].f.uchV3;
            bb += p_vTuples[i].b.uchV1;
            bg += p_vTuples[i].b.uchV2;
            br += p_vTuples[i].b.uchV3;
            sf += p_vTuples[i].sigmaf;
            sb += p_vTuples[i].sigmab;
        }
        
        fb /= (num + 1e-10);
        fg /= (num + 1e-10);
        fr /= (num + 1e-10);
        bb /= (num + 1e-10);
        bg /= (num + 1e-10);
        br /= (num + 1e-10);
        sf /= (num + 1e-10);
        sb /= (num + 1e-10);
        
        Scalar fc = Scalar(fb, fg, fr);
        Scalar bc = Scalar(bb, bg, br);
        Scalar pc = LOAD_RGB_SCALAR(data, xi*p_nStep + yj*p_nChannels);
        double   df = p_cMattingFunc.colorDistance2(pc, fc);
        double   db = p_cMattingFunc.colorDistance2(pc, bc);
        Scalar tf = fc;
        Scalar tb = bc;
        
        int index = xi * p_nWidth + yj;
        if (df < sf)
        {
            fc = pc;
        }
        if (db < sb)
        {
            bc = pc;
        }
        if (fc.uchV1 == bc.uchV1 && fc.uchV2 == bc.uchV2 && fc.uchV3 == bc.uchV3)
        {
            p_vFtuples[index].confidence = 0.00000001;
        }
        else
        {
            p_vFtuples[index].confidence = exp(-10 * p_cMattingFunc.chromaticDistortion(xi, yj, tf, tb));
        }
        
        
        p_vFtuples[index].f = fc;
        p_vFtuples[index].b = bc;
        
        
        p_vFtuples[index].alphar = max(0.0, min(1.0,p_cMattingFunc.comalpha(pc, fc, bc)));
    }
    p_vTuples.clear();
}

void SharedMatting::localSmooth()
{
    vector<PixelPoint>::iterator it;
    double sig2 = 100.0 / (9 * 3.1415926);
    double r = 3 * sqrt(sig2);
    for (it = p_vUnknownSet.begin(); it != p_vUnknownSet.end(); ++it)
    {
        int xi = it->x;
        int yj = it->y;
        
        int i1 = max(0, int(xi - r));
        int i2 = min(int(xi + r), p_nHeight - 1);
        int j1 = max(0, int(yj - r));
        int j2 = min(int(yj + r), p_nWidth - 1);
        
        int indexp = xi * p_nWidth + yj;
        Ftuple ptuple = p_vFtuples[indexp];
        
        Scalar wcfsumup = Scalar(0, 0, 0);
        Scalar wcbsumup = Scalar(0, 0, 0);
        double wcfsumdown = 0;
        double wcbsumdown = 0;
        double wfbsumup   = 0;
        double wfbsundown = 0;
        double wasumup    = 0;
        double wasumdown  = 0;
        
        for (int k = i1; k <= i2; ++k)
        {
            for (int l = j1; l <= j2; ++l)
            {
                int indexq = k * p_nWidth + l;
                Ftuple qtuple = p_vFtuples[indexq];
                
                double d = p_cMattingFunc.pixelDistance(PixelPoint(xi, yj), PixelPoint(k, l));
                
                if (d > r)
                {
                    continue;
                }
                
                double wc;
                if (d == 0)
                {
                    wc = exp(-(d * d) / sig2) * qtuple.confidence;
                }
                else
                {
                    wc = exp(-(d * d) / sig2) * qtuple.confidence * abs(qtuple.alphar - ptuple.alphar);
                }
                wcfsumdown += wc * qtuple.alphar;
                wcbsumdown += wc * (1 - qtuple.alphar);
                
                wcfsumup.uchV1 += wc * qtuple.alphar * qtuple.f.uchV1;
                wcfsumup.uchV2 += wc * qtuple.alphar * qtuple.f.uchV2;
                wcfsumup.uchV3 += wc * qtuple.alphar * qtuple.f.uchV3;
                
                wcbsumup.uchV1 += wc * (1 - qtuple.alphar) * qtuple.b.uchV1;
                wcbsumup.uchV2 += wc * (1 - qtuple.alphar) * qtuple.b.uchV2;
                wcbsumup.uchV3 += wc * (1 - qtuple.alphar) * qtuple.b.uchV3;
                
                
                double wfb = qtuple.confidence * qtuple.alphar * (1 - qtuple.alphar);
                wfbsundown += wfb;
                wfbsumup   += wfb * sqrt(p_cMattingFunc.colorDistance2(qtuple.f, qtuple.b));
                
                double delta = 0;
                double wa;
                if (p_ppTriData[k][l] == 0 || p_ppTriData[k][l] == 255)
                {
                    delta = 1;
                }
                wa = qtuple.confidence * exp(-(d * d) / sig2) + delta;
                wasumdown += wa;
                wasumup   += wa * qtuple.alphar;
            }
        }
        
        int b, g, r;
        b = data[xi * p_nStep +  yj* p_nChannels];
        g = data[xi * p_nStep +  yj * p_nChannels + 1];
        r = data[xi * p_nStep +  yj * p_nChannels + 2];
        Scalar cp = Scalar(b, g, r);
        Scalar fp;
        Scalar bp;
        
        double dfb;
        double conp;
        double alp;
        
        bp.uchV1 = min(255.0, max(0.0,wcbsumup.uchV1 / (wcbsumdown + 1e-200)));
        bp.uchV2 = min(255.0, max(0.0,wcbsumup.uchV2 / (wcbsumdown + 1e-200)));
        bp.uchV3 = min(255.0, max(0.0,wcbsumup.uchV3 / (wcbsumdown + 1e-200)));
        
        fp.uchV1 = min(255.0, max(0.0,wcfsumup.uchV1 / (wcfsumdown + 1e-200)));
        fp.uchV2 = min(255.0, max(0.0,wcfsumup.uchV2 / (wcfsumdown + 1e-200)));
        fp.uchV3 = min(255.0, max(0.0,wcfsumup.uchV3 / (wcfsumdown + 1e-200)));
        
        //double tempalpha = comalpha(cp, fp, bp);
        dfb  = wfbsumup / (wfbsundown + 1e-200);
        
        conp = min(1.0, sqrt(p_cMattingFunc.colorDistance2(fp, bp)) / dfb) * exp(-10 * p_cMattingFunc.chromaticDistortion(xi, yj, fp, bp));
        alp  = wasumup / (wasumdown + 1e-200);
        
        double alpha_t = conp * p_cMattingFunc.comalpha(cp, fp, bp) + (1 - conp) * max(0.0, min(alp, 1.0));
        
        p_ppAlpha[xi][yj] = alpha_t * 255;
    }
    p_vFtuples.clear();
}





void SharedMatting::getMatte()
{
    p_puchOutPixmap = new unsigned char[p_nWidth * p_nHeight * 4];
    int nPixelPos = 0;
    int nPixelInPos = 0;

    for(int i=0; i<p_nHeight; i++)
    {
        for(int j=0; j<p_nWidth; j++)
        {
            nPixelPos = i*p_nWidth*4+j*4;
           nPixelInPos = i*p_nStep + j*p_nChannels;
 
//            p_puchOutPixmap[nPixelPos] = p_ppAlpha[i][j];
//            p_puchOutPixmap[nPixelPos+1]=p_ppAlpha[i][j];
//            p_puchOutPixmap[nPixelPos+2]=p_ppAlpha[i][j];
//            p_puchOutPixmap[nPixelPos+3]=255;
            p_puchOutPixmap[nPixelPos] = p_puchInPixmap[nPixelInPos];
            p_puchOutPixmap[nPixelPos+1] = p_puchInPixmap[nPixelInPos+1];
            p_puchOutPixmap[nPixelPos+2] = p_puchInPixmap[nPixelInPos+2];
            p_puchOutPixmap[nPixelPos+3] = p_ppAlpha[i][j];


        }
    }
}

