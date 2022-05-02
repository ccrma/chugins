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

private:
    int m_width; 
    int m_height;
    int m_channels;
    float m_currentColumn; // measured as percent of total width
    unsigned char *m_data;
    std::vector<float> m_columnWeights;
};

#endif