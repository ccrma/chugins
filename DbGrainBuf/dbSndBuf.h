#ifndef dbSndBuf_h
#define dbSndBuf_h 

#include <chuck_def.h> // defines SAMPLE, usually float
#include <cmath>
#include "AudioFile.h"
#include <chuck_errmsg.h>

class dbAudioFile : public AudioFile<SAMPLE>
{
    // can't override reportError since its a private method
};

class dbSndBuf
{
public:
    dbSndBuf(float sampleRate)
    {
        this->chuckSampleRate = sampleRate;
        this->loop = false;
        this->rateMult = 1.f;
        this->maxFilterRadius = 5;
        this->cleanup();
    }

    ~dbSndBuf()
    {
        this->cleanup();
    }

    int ReadHeader(std::string filename)
    {
        int err = 0;
        this->cleanup();
        if(this->audioFile.load(filename)) // loads entire file
        {
            this->numFrames = this->audioFile.getNumSamplesPerChannel();
            this->sampleRate = this->audioFile.getSampleRate();
            this->currentSample = (SAMPLE) 0;
            this->currentFrame = 0.;
            this->sampleRatio = this->sampleRate / this->chuckSampleRate;
            this->rateChanged();
        }
        else
        {
            err = 1;
        }
        return err;
    }

    void DumpToStdout()
    {
        this->audioFile.printSummary();
    }

    double GetLengthInSeconds()
    {
        return this->audioFile.getLengthInSeconds();
    }

    int GetNChan()
    {
        return this->audioFile.getNumChannels();
    }

    void SetRate(float x)
    {
        this->rateMult = x;
        this->rateChanged();
    }

    float GetRate()
    {
        return this->rateMult;
    }

    void SetLoop(bool l)
    {
        this->loop = l;
    }

    int GetLoop()
    {
        return this->loop;
    }

    void SetPosition(long pos)
    {
        this->currentFrame = pos;
    }

    void SetPhase(float pct) // 
    {
        if(this->numFrames == 0)
        {
            std::cerr << "dbSndBuf::SetPhase must be invoked after the file is read." 
            << std::endl;
        }
        this->currentFrame = this->numFrames * pct;
    }

    int GetPosition()
    {
        return (int) this->currentFrame;
    }

    float GetPhase()
    {
        return (this->currentFrame / this->numFrames);
    }

    int SetMaxFilt(int w)
    {
        this->maxFilterRadius = w;
        return w;
    }

    int GetMaxFilt()
    {
        return this->maxFilterRadius;
    }

    // This entrypoint is for "standalone" mode (ie: no grains).
    // In this scenario, there is one-true "cursor" and we're responsible
    // for the bookkeeping.
    SAMPLE Sample(int chan=0, bool andStep=true)
    {
        this->currentSample = this->Sample(chan, this->currentFrame, 
                                this->filterRadius);
        if(andStep)
        {
            this->currentFrame = this->handleEdge((float)
                    this->currentFrame + this->currentRate);
        }
        return this->currentSample;
    }

    SAMPLE Sample(int chan, double pos, int filterRadius)
    {
        SAMPLE s;
        long frame = this->calcFrame(static_cast<long>(pos), 0); 
        if(filterRadius <= 1)
        {
            float pct = pos - frame;
            long neighbor = this->calcFrame(frame, 1); // handles fwd and rev
            SAMPLE now = this->lookupSample(frame, chan);
            SAMPLE next = this->lookupSample(neighbor, chan);
            s = now + pct * (next - now);
        }
        else
        {
            // for now we'll do box a filter (avg under filter)
            SAMPLE sum = 0;
            int numSamps = 0;
            for(int i = -filterRadius; i <= filterRadius; i++)
            {
                long sframe = this->calcFrame(frame, i);
                sum += this->lookupSample(sframe, chan);
                numSamps++;
            }
            sum /= numSamps; // box filter
            s = sum;
        }
        return s;
    }

    int IsDone()
    {
        return !this->loop && 
            (this->currentFrame >= this->numFrames ||
            this->currentFrame < 0);
    }

private:
    SAMPLE lookupSample(unsigned long frame, unsigned chan=0)
    {
        return this->audioFile.samples[chan][frame];
    }

    unsigned long calcFrame(int frame, int steps)
    {
        if(!this->reverse)
            frame += steps;
        else
            frame -= steps;
        return this->handleEdge((int) frame);
    }

    int handleEdge(int f)
    {
        if(f >= this->numFrames)
        {
            if(this->loop)
                f -= this->numFrames;
            else
                f = this->numFrames-1;
        }
        else
        if(f < 0)
        {
            if(this->loop)
                f = this->numFrames - 1;
            else
                f = 0;
        }
        return f;
    }

    float handleEdge(float f)
    {
        if(f >= this->numFrames)
        {
            if(this->loop)
                f -= this->numFrames;
            else
                f = this->numFrames-1;
        }
        else
        if(f < 0)
        {
            if(this->loop)
                f = this->numFrames - 1;
            else
                f = 0;
        }
        return f;
    }

    void rateChanged()
    {
        this->currentRate = this->sampleRatio * this->rateMult;
        this->reverse = this->currentRate < 0.;
        this->filterRadius = 1 + std::abs(static_cast<int>(.5*this->currentRate));
        if(this->filterRadius > this->maxFilterRadius)
            this->filterRadius = this->maxFilterRadius;
        /*
        std::cout << "rateChanged: " << this->currentRate << " filter: " << 
            this->filterRadius << std::endl;
        */
    }

    void cleanup()
    {
        this->numFrames = 0;
        this->sampleRate = 0; // of the file
        this->currentFrame = 0.f;
        this->currentRate = 1.f;
        this->sampleRatio = 1.f; // ratio of filerate to chuckrate
        this->currentSample = (SAMPLE) 0.f;

        // this->audioFile.clearAudioBuffer(); private, happens on each load
    }

private:
    float chuckSampleRate;

    // sndfile state -------------
    dbAudioFile audioFile;
    long numFrames;
    uint32_t sampleRate; // of the file

    // playback state ------------
    bool loop; 
    float rateMult;       // specified by user
    double currentFrame;   // fractional in all but rate == 1 cases
                            // use of float means that we're limited to 
                            // framesizes < 2^24 (16.8M). Since we've
                            // gone to the trouble of unsigned long numFrames
                            // we use double for currentFrame which
                            // gives framesizes < 2^53
    float sampleRatio; // ratio of file and chuck

    //
    // currentRate combines sampleRatio and rateMult to give us a
    // ratio of inputSamples to outputSamples.  On each iteration we
    // increase the current frame by this amount.
    //
    // * > 1 we should downsample multiple/arbitrary inputs 
    //  to produce a single output. For linear, a box-filter would produce 
    //  the avg value. A "tent" filter would like be a better LPF.
    //  Both approaches might be described as linear.
    //
    //  * < 1 we can simply interpolate between a fixed number 
    //   of neighbors (2 for lerp) to produce one output.
    //
    float currentRate;    
    int filterRadius; // int(abs(.5*currentRate)) + 1)
    int maxFilterRadius;
    bool reverse;
    SAMPLE currentSample;
};

#endif