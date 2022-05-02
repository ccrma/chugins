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

#include "concurrentQ.h"
#include "fftsg.h"
#include "ringbuffer.h"
#include "image.h"

#include <thread>
#include <mutex>

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
    void Init(int computeSize, int overlap);
    void LoadSpectralImage(char const *filename); // asynchronous
    float Tick(float in);

private:
    float m_sampleRate;

    static const int k_MaxComputeSize = 4096; // fftsize
    static void workThreadFunc(DbSpectral *);
    static void loadThreadFunc(DbSpectral *);

    // inputBuffer: only accessed by audio thread. We copy-out when
    // a compute-size chunk is available, then reuse some of the input
    // for the next compute according to overlap.  Ringbuffer just helps
    // iron out the edge conditions.
    Ringbuffer<FFTSg::t_Sample,  2 * k_MaxComputeSize> m_inputBuffer;

    // outputBuffer: producer is workThread, consumer is audioThread.
    // When a compute-size chunk is produced, accumulate over compute-sized
    // contents.
    Ringbuffer<FFTSg::t_Sample,  2 * k_MaxComputeSize> m_outputBuffer;

    int m_computeSize;
    int m_overlap;
    std::vector<FFTSg::t_Sample> m_computeBuf;

    FFTSg m_fft;
    class dbWindowing const *m_window;

    float m_spectralRate; // to loop update spectralTime
    float m_spectralTime; // measured in pct-width
    int m_verbosity;

    std::thread::id m_mainThreadId, m_workThreadId, m_loadThreadId;
    std::thread m_loadThread;
    std::thread m_workThread;
    ConcurrentQ<std::function<void()>> m_loadQueue;
    ConcurrentQ<std::function<void()>> m_workQueue;

    void loadImage(std::string filename); // in loadThread
    void doWork(FFTSg::t_Sample *computeBuff, int bufSize);

    std::mutex m_loadImageLock;
    SpectralImage *getImage();
    SpectralImage *m_spectralImage;
    void setImage(SpectralImage *);
};

#endif