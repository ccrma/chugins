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
    m_fftSize(512), // Init called from main.cpp, so this value doesn't matter
    m_overlap(128),
    m_scanTime(0.f),
    m_scanRate(100), /* columns per second */
    m_scanRatePct(0.), /* computed after we know image width */
    m_currentColumn(0), // depends on image
    m_verbosity(0),
    m_mode(k_EQOnly)
{
    m_mainThreadId = std::this_thread::get_id();
    m_workThread = std::thread(workThreadFunc, this);
    m_loadThread = std::thread(loadThreadFunc, this);
    m_window = dbWindowing::Get(dbWindowing::k_Hann);
    m_freqRange[0] = m_nextFreqRange[0] = 100;
    m_freqRange[1] = m_nextFreqRange[1] = 4000;
    m_delayMax = 0.f;
    m_feedbackMin = 0.f;
    m_feedbackMax = 0.f;
    this->updateScanRate();
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
DbSpectral::Init(int fftSize, int overlap, ImgModes m)
{
    m_mode = m;
    m_fftSize = std::min(fftSize, k_MaxFFTSize);
    m_overlap = overlap;
    m_decimation = m_fftSize / m_overlap;
    m_fft.InitPlan(m_fftSize, true/*real*/);
    m_computeBuf.resize(m_fftSize);
    for(int i=0;i<m_fftSize;i++)
        m_computeBuf[i] = 0.f;
    // Initialize the outputbuf with 0's 'til valid values start flowing.
    // If the compute is free we need m_fftSize worth of initial samples.
    // But it's not free.  If the loadavg if 1 we need 2*_computeSize.
    m_outputBuffer.producerClear();
    int startupSamps = m_fftSize + (m_fftSize >> 1);
    for(int i=0;i<startupSamps;i++)
        m_outputBuffer.insert(0.);
    assert(m_outputBuffer.readAvailable() == startupSamps);
}

void
DbSpectral::SetFreqMin(int m)
{
    m_nextFreqRange[0] = m; // careful not to change m_freqRange out from under Tick
    makeDirty();
}

void
DbSpectral::SetFreqMax(int m)
{
    m_nextFreqRange[1] = m;
    makeDirty();
}

void
DbSpectral::SetDelayMax(float x)
{
    m_delayMax = x;
}

void
DbSpectral::SetFeedbackMin(float x)
{
    m_feedbackMin = x;
}

void
DbSpectral::SetFeedbackMax(float x)
{
    m_feedbackMax = x;
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
        size_t numRead = m_inputBuffer.readBuff(computeBuf, m_fftSize, m_overlap);
        std::function<void()> fn = std::bind(&DbSpectral::doWork, this,
                                            computeBuf, m_fftSize);
        m_workQueue.Push(fn);
    }
    float out;
    if(!m_outputBuffer.remove(&out, 0.f))
        out = 0.f;
    checkDeferredUpdate();
    return out;
}

void
DbSpectral::LoadSpectralImage(char const *filename)
{
    std::string nm(filename);
    std::function<void()> fn = std::bind(&DbSpectral::loadImage, 
                                        this, nm/*pass by value*/);
    m_loadQueue.Push(fn);
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

float
DbSpectral::GetColumnPct()
{
    return m_scanTime;
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

/* We anticipate multiple edits arriving in clumps and want
 * to limit the reload-image frequency.  If we haven't computed
 * "for a while", we schedule a updateRequest for later. Otherwise
 * the outstanding request will still be in play and it's serviced
 * during Tick.
 */
void
DbSpectral::makeDirty()
{
    if(m_imgfilePath.size() == 0) return; // still loading image

    auto now = std::chrono::steady_clock::now();
    if(m_scheduledUpdate == m_zeroInstant)
    {
        // std::cerr << "Scheduling update\n";
        m_scheduledUpdate = now + std::chrono::seconds(2);
    }
}

void
DbSpectral::checkDeferredUpdate()
{
    if(m_scheduledUpdate == m_zeroInstant)
        return;

    auto now = std::chrono::steady_clock::now();
    if(now >= m_scheduledUpdate)
    {
        // std::cerr << "Applying scheduled update\n";
        if(m_imgfilePath.size() > 0)
            this->LoadSpectralImage(m_imgfilePath.c_str());
        m_scheduledUpdate = m_zeroInstant;
    }
}

void
DbSpectral::loadImage(std::string nm)
{
    assert(std::this_thread::get_id() == m_loadThreadId);
    SpectralImage *newImg = new SpectralImage();
    // FFTSize produces FFTSize/2 freqs so we request image resampling
    // (either up or down) to match this.  Keep in mind that we wish
    // to apply the image height to a subrange of frequencies (eg: 100, 4000).
    //
    // Example: 1024 FFT, 512 complex frequencies, 512 pixel weights.
    //  Equidistant freq bins: Nyquist/512 == 43 hz per bin
    //  Frequency range: 100-4000 (indices)
    //     3900/43 = 90 bins (5.6 pixels per bin, ie over-expressed)
    //    10000/43 = 232 bins (2ish pixels per bin)
    int totalbins = m_fftSize / 2;
    float nyquist = .5f*this->m_sampleRate;
    float freqPerBin = nyquist / totalbins;
    int freqBins = 1 + (m_nextFreqRange[1] - m_nextFreqRange[0]) / freqPerBin;
    int err = newImg->LoadFile(nm.c_str(), freqBins);
    if(!err)
        this->setImage(newImg, nm, freqPerBin, freqBins);
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
    m_fft.RealFFT(computeBuf); // samples are interleaved complex

    int nfreq = computeSize / 2;

    { // scope for access to image
        std::lock_guard<std::mutex> lock(m_loadImageLock);
        assert(m_scanTime >= 0.f && m_scanTime <= 1.f);
        if(this->m_spectralImage)
        {
            const float *eqW = m_spectralImage->GetColumnWeights(
                                    m_scanTime, &m_currentColumn, 0);
            const float *delayW = m_mode == k_EQOnly ? nullptr :
                                    m_spectralImage->GetColumnWeights(
                                     m_scanTime, &m_currentColumn, 1);
            const float *fbW = m_mode != k_EQDelayFeedback ? nullptr :
                                    m_spectralImage->GetColumnWeights(
                                     m_scanTime, &m_currentColumn, 2);
            float *complex = computeBuf; // interleaved
            if(!delayW || m_delayMax == 0.f)
            {
                int nbin = 0;
                float freq = 0.f;
                for(int i=0;i<nfreq;i++)
                {
                    if(freq < m_freqRange[0] || freq > m_freqRange[1])
                        complex += 2;
                    else
                    {
                        float eq = *eqW++;
                        eq *= eq; // optional exp-gain
                        *complex++ *= eq;
                        *complex++ *= eq;
                        nbin++;
                    }
                    freq += m_freqPerBin;
                }
                assert(nbin <= m_freqBins);
            }
            else
            {
                int nbin = 0;
                float freq = 0.f;
                float delayR, delayI;
                float fb = m_feedbackMax;
                float df = m_feedbackMax - m_feedbackMin;
                int first = false;
                for(int i=0;i<nfreq;i++)
                {
                    if(freq >= m_freqRange[0] && freq <= m_freqRange[1])
                    {
                        // delayTime is either time-varying or constant.
                        // In the varying case, even if the current value is
                        // 0 we must put-sample for the potentially non-zero
                        // value later.
                        float delayTime = delayW ? (m_delayMax * *delayW++) : m_delayMax;
                        int delaySamps = (int) (delayTime * m_sampleRate / m_decimation);

                        float nowR = complex[0];
                        float nowI = complex[1];

                        float eq = *eqW++;
                        eq *= eq; // optional exp-gain
                        #if 1
                            m_delayTable.PutSamp(nbin, nowR, nowI);
                            m_delayTable.GetSamp(nbin, &delayR, &delayI, delaySamps);
                            complex[0] = nowR + delayR;
                            complex[1] = nowI + delayI;
                        #else
                        if(delaySamps == 0)
                        {
                            m_delayTable.PutSamp(nbin, nowR, nowI);
                            complex[0] = nowR * eq;
                            complex[1] = nowI * eq;
                        }
                        else
                        {
                            if(!first)
                            {
                                // eg: 21, 864
                                // std::cerr << i << " delay " << delaySamps << "\n";
                                // std::cerr << i << " head " << m_delayTable.GetHead(nbin) << "\n";
                                first = true;
                            }
                            m_delayTable.GetSamp(nbin, &delayR, &delayI, delaySamps);
                            if(fbW)
                                fb = m_feedbackMin + df * *fbW++;
                            if(delayR != 0.)
                            {
                                std::cerr << i << " delay " << delayR << " " << delayI << "\n";
                            }

                            // feedback: combine nowR with delayR before putting
                            m_delayTable.PutSamp(nbin, nowR + fb*delayR, nowI + fb*delayI);

                            complex[0] = delayR * eq; // this is postEQ
                            complex[1] = delayI * eq;
                        }
                        #endif
                        nbin++;
                    }
                    freq += m_freqPerBin;
                    complex += 2;
                }
                assert(nbin <= m_freqBins);
            }
        }
    }

    m_fft.RealIFFT(computeBuf);
    m_window->Apply(computeBuf, computeSize, rescale);
    // now we must accumulate all these samples onto the output.
    // we only step by overlap on each unit of work
    m_outputBuffer.writeBuff(computeBuf, computeSize, m_overlap, true/*accum*/);

    m_scanTime += m_scanRatePct;
    if(m_scanTime > 1.f)
        m_scanTime = fmodf(m_scanTime, 1.f);
}

// happens after loading
void
DbSpectral::setImage(SpectralImage *i, std::string &nm, 
    float freqPerBin, int nbins) 
{
    std::lock_guard<std::mutex> lock(m_loadImageLock);
    if(m_spectralImage)
        delete m_spectralImage;

    m_spectralImage = i;
    m_imgfilePath = nm;
    m_freqPerBin = freqPerBin;
    m_freqBins = nbins;
    m_freqRange[0] = m_nextFreqRange[0];
    m_freqRange[1] = m_nextFreqRange[1];
    this->updateScanRate();
    if(m_verbosity)
    {
        std::cerr << "Spectral '" <<  m_spectralImage->GetName() 
            << "' ready. Resampled to h:"  
            << i->GetHeight() << " w: " << i->GetWidth() << "\n";
        std::cerr <<  "Spectral bins: " << nbins << ", Hz/bin: " << m_freqPerBin 
            << ", Range: " << m_freqRange[0] << "-" << m_freqRange[1] 
            << ", FFT: " << m_fftSize << ", Overlap:" << m_overlap << "\n";
    }
    if(m_mode != k_EQOnly)
    {
        int delayMaxSamps = (m_delayMax * m_sampleRate) / m_decimation;
        m_delayTable.Resize(m_freqBins, delayMaxSamps+1);
        if(m_verbosity)
            std::cerr << "Spectral delay: " << delayMaxSamps << "\n";
    }
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
        if(p->m_verbosity > 2)
        {
            std::cerr << "workThread delegateBegin " 
                     << p->m_workQueue.Size() << "-------------------\n";
        }
        item();  // <------------- work!
        if(p->m_verbosity > 2)
        {
            std::cerr << "workThread delegateEnd active:" 
                     << p->m_workQueue.IsActive() << "---------------------\n";
        }
    }
    if(p->m_verbosity > 1)
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