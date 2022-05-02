#ifndef spectralimage_h
#define spectralimage_h

#include <vector>

class SpectralImage
{
public:
    SpectralImage();
    ~SpectralImage();

    int LoadFile(char const *filename, int resizeY=0);
    float const *GetColumnWeights(float xPct);
    char const *GetName() { return m_imageName.c_str(); }

private:
    int m_width; 
    int m_height;
    int m_channels;
    int m_currentCol; // XXX: could be float and interpolate, for now point-sample
    unsigned char *m_data;
    std::vector<float> m_columnWeights;
    std::string m_imageName;
};

#endif