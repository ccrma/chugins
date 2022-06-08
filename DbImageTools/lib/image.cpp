#include "image.h"

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

#include <iostream>
#include <cstring>

/* -------------------------------------------------------------------- */
Image::Image() :
    m_data(nullptr)
{}

Image::~Image()
{
    if(m_data)
        stbi_image_free(m_data);
}

int
Image::LoadFile(char const *filename, bool verbose)
{
    int err = 0;
    if(m_data != nullptr)
        stbi_image_free(m_data);

    m_data = stbi_load(filename, &m_width, &m_height, &m_channels, 0);
    if(m_data)
    {
        char const *base = strrchr(filename, '/');
        if(!base)
            base = strrchr(filename, '\\');
        if(base) 
            base++;
        else 
            base = filename;
        m_imageName = base;
        if(verbose)
        {
            std::cerr << m_imageName.c_str() << " opened w, h, nch: " 
                    << m_width << ", " << m_height << ", " << m_channels <<"\n";
        }
    }
    else
        err = -1;
    return err;
}

inline void clamp(float &a)
{
    if(a < 0.f)
        a = 0.f;
    else
    if(a > 1.f)
        a = 1.f;
}

int
Image::GetSample(float x, float y, float *r, float *g, float *b, float *a)
{
    int err;
    if(m_data != nullptr)
    {
        clamp(x);
        clamp(y);
        int ix = (m_width-1) * x;
        int iy = (m_height-1) * y; 

        // std::cerr << "sampling " << ix << ", " << iy << "\n";
        // invert y (?)
        // iy = (m_height-1) - iy;

        unsigned char *pixel = m_data + m_channels * ix;
        int rowStride = m_width * m_channels;
        pixel += iy * rowStride;

        *r = pixel[0] / 255.f;
        *g = pixel[1] / 255.f;
        *b = pixel[2] / 255.f;
        *a = (m_channels >= 4) ? pixel[3] / 255.f : 1.f;;

        err = 0;
    }
    else
        err = 1;
    return err;
}

/* -------------------------------------------------------------------- */
SpectralImage::SpectralImage()
{
}

SpectralImage::~SpectralImage()
{
}

int
SpectralImage::LoadFile(char const *filename, int resizeY, bool verbose)
{
    int err = Image::LoadFile(filename, verbose);
    if(!err)
    {
        m_columnWeights.resize(resizeY*m_channels);
        if(m_height != resizeY)
        {
            size_t sz = m_width*resizeY*m_channels;
            unsigned char *output = (unsigned char *) stbi__malloc(sz);
            int flags = 0; // for alpha handling
            int rerr = stbir_resize_uint8_srgb(m_data, m_width, m_height, 0 /*stride*/,
                                    output, m_width, resizeY, 0 /*stride*/,
                                    m_channels, m_channels==4?3:0, flags);
            if(rerr == 0)
            {
                err = 1;
                std::cerr << m_imageName.c_str() << 
                    " problem resizing to y:" << resizeY << "\n";
            }
            else
            {
                if(m_data)
                    stbi_image_free(m_data);
                m_data = output;
                m_height = resizeY;
                if(verbose)
                {
                    std::cerr << m_imageName.c_str() <<
                    " resized to y:" << resizeY << "\n";
                }
            }
        }
    }
    else
        err = -1;
    return err;
}

float const *
SpectralImage::GetColumnWeights(float xPct, int *colno, int ch)
{
    if(m_columnWeights.size() > 0)
    {
        // std::cerr << "sample spectrum at " << xPct << "\n";
        int col = xPct * (m_width-1) + .5f;
        if(col != m_currentCol)
        {
            // Current scheme is worst-case for cache. If we pre-rotate 
            // the image we might be in better shape, but that's premature.
            // Here we also invert Y since we want high frequencies on the
            // image top.
            int rowStride = m_width * m_channels;
            unsigned char *pixel = m_data + m_channels * col;
            for(int i=m_height-1;i>=0;i--)
            {
                // m_columnWeights[i] = ((int)pixel[0] + pixel[1] + pixel[2]) / (255*3.);
                m_columnWeights[i] = pixel[0] / 255.f;
                if(m_channels > 1)
                {
                    m_columnWeights[i+m_height] = pixel[1] / 255.f;
                    if(m_channels > 2)
                    {
                        m_columnWeights[i+2*m_height] = pixel[2] / 255.f;
                        if(m_channels > 3)
                            m_columnWeights[i+3*m_height] = pixel[3] / 255.f;
                    }
                }
                pixel += rowStride;
            }
            m_currentCol = col;

            // std::cerr << "New table: " << col << "\n";
        }
        *colno = m_currentCol;
        return &m_columnWeights[0] + m_height * ch;
    }
    else
        return nullptr;
}
