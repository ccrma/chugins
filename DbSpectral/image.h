#ifndef spectralimage_h
#define spectralimage_h

#include <vector>
#include <string>

class SpectralImage
{
public:
    SpectralImage();
    ~SpectralImage();

    int LoadFile(char const *filename, int resizeY=0, bool verbose=false);
    float const *GetColumnWeights(float xPct, int *column, int chan);

    char const *GetName() { return m_imageName.c_str(); }
    int GetWidth() { return m_width; }
    int GetHeight() { return m_height; }

private:
    int m_width; 
    int m_height;
    int m_channels;
    int m_currentCol; // XXX: could be float and interpolate, for now point-sample
    unsigned char *m_data;
    std::vector<float> m_columnWeights; // size is nchans * resizeY
    std::string m_imageName;
};

#endif