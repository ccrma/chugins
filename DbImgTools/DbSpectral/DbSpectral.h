#ifndef DbSpectral_h
#define DbSpectral_h
/**
 *
 * Copyright (c) 2022 Dana Batali
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include "../lib/concurrentQ.h"
#include "../lib/fftsg.h"
#include "../lib/ringbuffer.h"
#include "../lib/image.h"
#include "../lib/complexDelay.h"

#include <thread>
#include <mutex>
#include <chrono>
#include <functional>

/**
 * @brief DbSpectral is a time-varying mono audio filter parameterized by
 * an image file.  DbSpectral reads image files (.png) representing
 * a spectrogram.  Image y is interpretted as frequency from
 * low (y=0) to Nyquist.  Image x is interpetted as time, the value at
 * image(x,y) is interpretters as a gain for incoming FFT samples.
 * 
 * We convert the incoming signal to the frequency domain, multiply by our
 * current spectrum then convert back to time domain. We loop through the
 * spectragram at at (potentially variable) rate.  New spectrograms can be 
 * loaded on-the-fly but take effect when the asynchronous file-read is 
 * complete.
 * 
 * Files are loaded in a separate thread to prevent hiccups in the
 * audio/realtime thread.
 */
 
class DbSpectral
{
public:
    DbSpectral(float sampleRate);
    ~DbSpectral();
    enum ImgModes
    {
        k_EQOnly, // R
        k_EQDelay, // RG
        k_EQDelayFeedback, // RGB
    };
    void Init(int computeSize, int overlap, ImgModes mode=k_EQOnly);
    void LoadSpectralImage(char const *filename); // asynchronous
    float Tick(float in);
    void SetScanRate(int rate); // image columns per second
    void SetScanMode(int mode); // 0: fwdloop, 1: revloop, 2: fwdrevloop
    int GetColumn();
    float GetColumnPct();
    void SetFreqMin(int min);
    void SetFreqMax(int max);
    void SetDelayMax(float max); // seconds
    void SetFeedbackMax(float max);
    void SetVerbosity(int v) { m_verbosity = v; }
    void SetMix(float mix);

private:
    float m_sampleRate;
    std::string m_imgfilePath;
    int m_verbosity;
    ImgModes m_mode;
    float m_delayMax;
    float m_feedbackMax;

    #define k_MaxFFTSize 4096
    static void workThreadFunc(DbSpectral *);
    static void loadThreadFunc(DbSpectral *);

    // inputBuffer: only accessed by audio thread. We copy-out to computeBuf
    // when compute-size chunk is available, then reuse some of the 
    // (untouched) input for the next compute according to overlap.  
    // Ringbuffer just helps iron out the edge conditions.
    Ringbuffer<FFTSg::t_Sample,  2 * k_MaxFFTSize> m_inputBuffer;
    std::vector<FFTSg::t_Sample> m_computeBuf;

    // outputBuffer: producer is workThread, consumer is audioThread.
    // When a compute-size chunk is produced, we *accumulate* over 
    // the compute-sized contents.
    Ringbuffer<FFTSg::t_Sample,  2 * k_MaxFFTSize> m_outputBuffer;

    float m_scanTime; // measured in pct-width (ie [0, 1])
    int m_scanRate;   // measured in cols per second
    float m_scanRatePct; // derived from m_scanRate and current image, pct per sample
    int m_currentColumn;

    // m_freqRange/m_freqPerBin is associated with loaded image.
    int m_freqRange[2], m_nextFreqRange[2];
    float m_freqPerBin;
    int m_freqBins;
    
    using t_Instant = std::chrono::time_point<std::chrono::steady_clock>;
    using t_Duration = std::chrono::duration<unsigned int, std::nano>;
    t_Instant m_scheduledUpdate;
    t_Instant m_zeroInstant;

    FFTSg m_fft;
    int m_fftSize; // samples, typical 2048
    int m_overlap; // samples, typical 512
    class Windowing const *m_window;
    float m_mix;

    std::thread::id m_mainThreadId, m_workThreadId, m_loadThreadId;
    std::thread m_loadThread;
    std::thread m_workThread;
    ConcurrentQ<std::function<void()>> m_loadQueue;
    ConcurrentQ<std::function<void()>> m_workQueue;

private:
    void updateScanRate();
    void makeDirty();
    void checkDeferredUpdate();

private: // FFT calcs
    void doWork(FFTSg::t_Sample *computeBuff, int bufSize);

private: // image loading
    void loadImage(std::string filename); // in loadThread
    SpectralImage *getImage();
    void setImage(SpectralImage *, std::string &nm, float freqPerBin, int freqBins);
    int getDelaySamps(float seconds);

    std::mutex m_loadImageLock;
    SpectralImage *m_spectralImage;
    ComplexDelayTable m_delayTable;
};

#endif