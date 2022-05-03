/* see comments in README.md */
#define _USE_MATH_DEFINES
#include "DbSpectral.h"
#include "dbWindowing.h"
#include "fftsg.h"

#include <iostream>
#include <functional>
#include <cassert>
#include <string>

DbSpectral::DbSpectral(float sampleRate) :
    m_sampleRate(sampleRate),
    m_spectralImage(nullptr),
    m_computeSize(512),
    m_overlap(128),
    m_scanTime(0.f),
    m_scanRate(100), /* columns per second */
    m_scanRatePct(0.), /* computed after we know image width */
    m_currentColumn(0), // depends on image
    m_verbosity(1)
{
    m_mainThreadId = std::this_thread::get_id();
    m_workThread = std::thread(workThreadFunc, this);
    m_loadThread = std::thread(loadThreadFunc, this);
    m_window = dbWindowing::Get(dbWindowing::k_Hann);
}

DbSpectral::~DbSpectral()
{ 
    if(m_verbosity)
        std::cerr << "DbSpectral cleanup\n";
    
    if(m_workThread.joinable())
    {
        m_workQueue.Bail();
        m_workThread.join();
    }
    if(m_loadThread.joinable())
    {
        m_loadQueue.Bail();
        m_loadThread.join();
    }

    if(m_spectralImage)
        delete m_spectralImage;
}

void
DbSpectral::Init(int computeSize, int overlap)
{
    m_computeSize = computeSize;
    m_overlap = overlap;
    m_fft.InitPlan(m_computeSize, true/*real*/);
    m_computeBuf.resize(m_computeSize);
    for(int i=0;i<m_computeSize;i++)
        m_computeBuf[i] = 0.f;
    // Initialize the outputbuf with 0's 'til valid values start flowing.
    // If the compute is free we need m_computeSize worth of initial samples.
    // But it's not free.  If the loadavg if 1 we need 2*_computeSize.
    m_outputBuffer.producerClear();
    int startupSamps = m_computeSize + (m_computeSize >> 1);
    for(int i=0;i<startupSamps;i++)
        m_outputBuffer.insert(0.);
    assert(m_outputBuffer.readAvailable() == startupSamps);
}

float
DbSpectral::Tick(float in)
{
    // We buffer input 'til m_inputBuffer contains one unit of work.
    // Then we async-compute our filtered results into the output buffer.
    // Latency is m_fft.Size()/sampleRate + fftTime.
    // fftTime must be << m_fft.Size()/sampleRate.
    m_inputBuffer.insert(in);
    size_t avail = m_inputBuffer.readAvailable(); 
    if(avail == m_fft.Size())
    {
        FFTSg::t_Sample *computeBuf = &m_computeBuf[0];
        // here we read computeSize, but move head by overlap (eg 512, 128)
        size_t numRead = m_inputBuffer.readBuff(computeBuf, m_computeSize, m_overlap);
        std::function<void()> fn = std::bind(&DbSpectral::doWork, this,
                                            computeBuf, m_computeSize);
        m_workQueue.Push(fn);
    }
    float out;
    if(!m_outputBuffer.remove(&out, 0.f))
        out = 0.f;
    return out;
}

void
DbSpectral::LoadSpectralImage(char const *filename)
{
    std::string nm(filename);
    std::function<void()> fn = std::bind(&DbSpectral::loadImage, 
                                        this, nm/*pass by value*/);
    m_loadQueue.Push(fn);
    this->updateScanRate();
}

void
DbSpectral::SetScanRate(int rate)
{
    this->m_scanRate = rate;
    this->updateScanRate();
}

int
DbSpectral::GetColumn()
{
    return m_currentColumn;
}

void
DbSpectral::updateScanRate()
{
    // potential race? when loading an image? (vs user setting scanrate?)
    int imageWidth = m_spectralImage ? m_spectralImage->GetWidth() : 512; 
    float pctPerSecond = m_scanRate / (float) imageWidth;

    // the columns weights are currently updated once per unit-of work
    // for FFT of 512 with overlap of 128:  128 samples / (samples/Second)
    float secondsPerUpdate = m_overlap / m_sampleRate; // seconds
    m_scanRatePct = pctPerSecond * secondsPerUpdate;

    // std::cerr << "updateScanRate " << m_scanRate << " => " << m_scanRatePct << "\n";
}

void
DbSpectral::SetScanMode(int mode)
{
    if(mode >= 0 && mode < k_ScanModeCount)
        this->m_scanMode = (ScanMode) mode;
}

void
DbSpectral::loadImage(std::string nm)
{
    assert(std::this_thread::get_id() == m_loadThreadId);
    SpectralImage *newImg = new SpectralImage();
    int err = newImg->LoadFile(nm.c_str(), m_computeSize/2); // FFTSize produces FFTSize/2 freqs
    if(!err)
        this->setImage(newImg);
    else
        std::cerr << "DbSpectral problem opening file " << nm << "\n";
}

void 
DbSpectral::doWork(FFTSg::t_Sample *computeBuf, int computeSize) // in workthread
{
    assert(std::this_thread::get_id() == m_workThreadId);
    // rescale: computeSize since fft/ifft isn't normalized
    //  3/2 due to windowing.
    const float rescale = 1.f / (1.5f * computeSize); 

    m_window->Apply(computeBuf, computeSize);
    m_fft.RealFFT(computeBuf); // half of the output samps are real, half imag

    int nfreq = computeSize / 2;

    { // scope for access to image
        std::lock_guard<std::mutex> lock(m_loadImageLock);
        assert(m_scanTime >= 0.f && m_scanTime <= 1.f);
        float const *spectralWeights = this->m_spectralImage ? 
                    this->m_spectralImage->GetColumnWeights(m_scanTime, 
                                                        &m_currentColumn) 
                    : nullptr;
        if(spectralWeights)
        {
            float *real=computeBuf;
            float *imag=computeBuf+nfreq;
            for(int i=0; i<nfreq; i++)
            {
                float w = *spectralWeights++;
                *real++ *= w;
                *imag++ *= w;
            }
        }
    }

    m_fft.RealIFFT(computeBuf);
    m_window->Apply(computeBuf, computeSize, rescale);
    // now we must accumulate all these samples onto the output.
    // we only step by overlap on each unit of work
    m_outputBuffer.writeBuff(computeBuf, computeSize, m_overlap, true/*accum*/);

    m_scanTime += m_scanRatePct;
    if(m_scanMode == k_ScanFwdRev)
    {
        if(m_scanTime < 0.f || m_scanTime > 1.f)
        {
            m_scanRatePct = -m_scanRatePct;
            m_scanTime += m_scanRatePct;
        }
    }
    if(m_scanTime > 1.f)
        m_scanTime = fmodf(m_scanTime, 1.f);
}

void
DbSpectral::setImage(SpectralImage *i)
{
    std::lock_guard<std::mutex> lock(m_loadImageLock);
    if(m_spectralImage)
        delete m_spectralImage;
    m_spectralImage = i;
    this->updateScanRate();

    // std::cerr << m_spectralImage->GetName()  << " ready to roll.\n";
}

SpectralImage *
DbSpectral::getImage()
{
    std::lock_guard<std::mutex> lock(m_loadImageLock);
    return m_spectralImage;
}

/* static */ void
DbSpectral::workThreadFunc(DbSpectral *p)
{
    p->m_workThreadId = std::this_thread::get_id();
    while(p->m_workQueue.IsActive())
    {
        auto item = p->m_workQueue.Pop(); // should block when empty
        if(p->m_verbosity > 1)
        {
            std::cerr << "workThread delegateBegin " 
                     << p->m_workQueue.Size() << "-------------------\n";
        }
        item();  // <------------- work!
        if(p->m_verbosity > 1)
        {
            std::cerr << "workThread delegateEnd active:" 
                     << p->m_workQueue.IsActive() << "---------------------\n";
        }
    }
    if(p->m_verbosity)
        std::cerr << "workThread exiting\n";
}

/* static */ void
DbSpectral::loadThreadFunc(DbSpectral *p)
{
    p->m_loadThreadId = std::this_thread::get_id();
    while(p->m_loadQueue.IsActive())
    {
        auto item = p->m_loadQueue.Pop(); // should block when empty
        if(p->m_verbosity > 1)
        {
            std::cerr << "loadThread delegateBegin " 
                     << p->m_loadQueue.Size() << "-------------------\n";
        }
        item();  // <------------- load!
        if(p->m_verbosity > 1)
        {
            std::cerr << "loadThread delegateEnd active:" 
                     << p->m_loadQueue.IsActive() << "---------------------\n";
        }
    }
    if(p->m_verbosity)
        std::cerr << "loadThread exiting\n";
}