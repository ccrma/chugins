#ifndef image_h
#define image_h

#include <vector>
#include <string>

class Image
{
public:
    Image();
    virtual ~Image();

    int LoadFile(char const *filename, bool verbose=false);
    char const *GetName() { return m_imageName.c_str(); }
    int GetWidth() { return m_width; }
    int GetHeight() { return m_height; }
    int GetSample(float x, float y, float *r, float *g, float *b, float *a);

protected:
    std::string m_imageName;
    unsigned char *m_data;
    int m_width; 
    int m_height;
    int m_channels;
};

class SpectralImage : public Image
{
public:
    SpectralImage();
    ~SpectralImage();

    int LoadFile(char const *filename, int resizeY, bool verbose=false);
    float const *GetColumnWeights(float xPct, int *column, int chan);

private:
    int m_currentCol; // XXX: could be float and interpolate, for now point-sample
    std::vector<float> m_columnWeights; // size is nchans * resizeY
};

#endif