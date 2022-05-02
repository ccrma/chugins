#ifndef fftsg_h
#define fftsg_h

#include <vector>

class FFTSg
{
public:
    using t_Sample = float;

#if 1
    using t_FFTReal = double; 
    #define COS cos
    #define SIN sin
    #define ATAN atan
#else
    using t_FFTReal = float; 
    #define COS cosf
    #define SIN sinf
    #define ATAN atanf
#endif

    enum Pow2
    {
        k_Invalid,
        k_64 = 64,
        k_128 = 128,
        k_256 = 256,
        k_512 = 512,
        k_1024 = 1024,
        k_2048 = 2048,
        k_4096 = 4096
    };

    FFTSg();
    ~FFTSg();

    // fftSize represents the length of the signal buffer(s). 
    void InitPlan(Pow2 fftSize, bool realInputs);
    void InitPlan(int fftSize, bool realInputs);
    int Size() { return (int) m_fftSize; }

    // RealFFT converts signal to n/2 real and n/2 imaginary freq samples.
    void RealFFT(t_Sample *fz1);
    void RealIFFT(t_Sample *fz1);

    void FFT(t_Sample *fz1, t_Sample *fz2);
    void IFFT(t_Sample *fz1, t_Sample *fz2);

private:
    void doFFT(t_Sample *fz1, t_Sample *fz2, int sgn);
    Pow2 m_fftSize;

private:
    bool m_realInputs;
    std::vector<int> m_bitrev;
    std::vector<t_FFTReal> m_costab;
    std::vector<t_FFTReal> m_buffer;
};

#endif