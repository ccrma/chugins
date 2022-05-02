#include "image.h"

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

#include <iostream>

/* -------------------------------------------------------------------- */
SpectralImage::SpectralImage() :
    m_data(nullptr)
{
}

SpectralImage::~SpectralImage()
{
    if(m_data)
        stbi_image_free(m_data);
}

int
SpectralImage::LoadFile(char const *filename, int resizeY)
{
    int err=0;
    if(m_data != nullptr)
        stbi_image_free(m_data);
    m_data = stbi_load(filename, &m_width, &m_height, &m_channels, 0);
    if(m_data)
    {
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
                std::cerr << filename << " problem resizing to y:" << resizeY << "\n";
            }
            else
            {
                if(m_data)
                    stbi_image_free(m_data);
                m_data = output;
                m_height = resizeY;
            }
        }
        if(!err)
        {
            std::cerr << filename << " opened w, h, nch: " 
                << m_width << ", " << m_height << ", " << m_channels <<"\n";
        }
    }
    else
        err = -1;
    return err;
}

float const *
SpectralImage::GetColumnWeights(float xPct)
{
    return nullptr; /* &m_columnWeights[0]; */
}
