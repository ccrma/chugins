
#define _USE_MATH_DEFINES
#include "DbSpectral.h"
#include "dbWindowing.h"
#include "fftsg.h"

#define STBI_ONLY_PNG
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <functional>

DbSpectral::DbSpectral(float sampleRate) :
    m_sampleRate(sampleRate),
    m_spectrogram(nullptr),
    m_computeSize(512),
    m_overlap(128),
    m_rate(0.f),
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
    static unsigned long tick = 0;
    // const std::lock_guard<std::mutex> lock(m_processingLock); // scoped
    tick++;
    if(!m_spectrogram && false)
        return in;
    else
    {
        // We buffer input 'til m_inputBuffer contains one unit of work.
        // Then we async-compute our filtered results into the output buffer.
        // Latency is m_fft.Size()/sampleRate + fftTime.
        // fftTime must be << m_fft.Size()/sampleRate.
        m_inputBuffer.insert(in);
        size_t avail = m_inputBuffer.readAvailable(); 
        if(avail == m_fft.Size())
        {
            if(m_verbosity > 1)
                std::cerr << "before compute: " << avail << " t:" << tick << "\n";
            FFTSg::t_Sample *computeBuf = &m_computeBuf[0];
            // here we read computeSize, but move head by overlap (eg 512, 128)
            size_t numRead = m_inputBuffer.readBuff(computeBuf, m_computeSize, m_overlap);
            std::function<void()> fn = std::bind(&DbSpectral::doWork, this,
                                                computeBuf, m_computeSize);
            m_workQueue.Push(fn);
        }
        float out;
        if(m_verbosity > 1)
            std::cerr << "out: " << m_outputBuffer.readAvailable() << "\n";
        if(!m_outputBuffer.remove(&out, 0.f))
        {
            std::cerr << "Out of data " << tick << "\n";
            out = 0.f;
        }
        return out;
    }
}

void
DbSpectral::LoadSpectogram(char const *filename) // asynchronous
{
    std::string nm(filename);
    std::function<void()> fn = std::bind(&DbSpectral::loadImage, 
                                        this, nm/*pass by value*/);
    m_loadQueue.Push(fn);
}

void
DbSpectral::loadImage(std::string nm)
{
    assert(std::this_thread::get_id() == m_loadThreadId);
    int x, y, nch;
    stbi_us *result = stbi_load_16(nm.c_str(), &x, &y, &nch, 3/*desired channels*/);
    if(result)
    {
        // convert 3-channel image to single float for speed doing work
        stbi_image_free(result);
    }
}

void 
DbSpectral::doWork(FFTSg::t_Sample *computeBuf, int computeSize) // in workthread
{
    assert(std::this_thread::get_id() == m_workThreadId);
    m_window->Apply(computeBuf, computeSize);
    m_fft.RealFFT(computeBuf); // half of the output samps are real, half imag
    // check for spectogram, multiply weights by 
    m_fft.RealIFFT(computeBuf);
    m_window->Apply(computeBuf, computeSize);

    // now we must accumulate all these samples onto the output.
    // we only step by overlap on each unit of work
    m_outputBuffer.writeBuff(computeBuf, computeSize, m_overlap, true/*accum*/);

    if(m_verbosity > 1)
        std::cerr << "doneFFT outbufSize: " << m_outputBuffer.readAvailable() << "\n";
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