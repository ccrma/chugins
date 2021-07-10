#ifndef dbSndBuf_h
#define dbSndBuf_h

#include <chuck_def.h> // defines SAMPLE, usually float
#include <cmath>

#define DB_SND_BUF_CHUNKSIZE 32768  /* 33/44 secs of 1 chan */

class dbSndBuf
{
public:
    dbSndBuf(float sampleRate);
    ~dbSndBuf();

    int ReadHeader(char const *filename);

    void SetRate(float x)
    {
        this->m_rateMult = x;
        this->rateChanged();
    }

    void SetLoop(bool l)
    {
        this->m_loop = l;
    }

    void SetPosition(float pct)
    {
    }

    SAMPLE Sample(int chan=0, bool andStep=true)
    {
        long frame = static_cast<long>(this->m_currentFrame); // preserve sign
        long nextFrame = this->calcFrame(frame, 1); // handles fwd and rev
        if(this->m_filterRadius <= 1)
        {
            float pct = this->m_currentFrame - frame;
            SAMPLE now = this->lookupSample(frame, chan);
            SAMPLE next = this->lookupSample(nextFrame, chan);
            this->m_currentSample =  now + pct * (next - now);
        }
        else
        {
            // for now we'll do box a filter (avg under filter)
            SAMPLE sum = 0;
            int numSamps = 0;
            for(int i = -this->m_filterRadius;
                i <= this->m_filterRadius; i++)
            {
                long sframe = this->calcFrame(frame, i);
                sum += this->lookupSample(sframe, chan);
                numSamps++;
            }
            sum /= numSamps; // box filter
            this->m_currentSample = sum;
        }
        if(andStep)
            this->m_currentFrame = nextFrame;
        return this->m_currentSample;
    }

    /*
    SAMPLE Sample(float pos, float rate)
    {
    }
    */

    int IsDone()
    {
        return !this->m_loop && 
            (this->m_currentFrame >= this->m_numFrames ||
            this->m_currentFrame < 0);
    }

private:
    SAMPLE lookupSample(unsigned long frame, unsigned chan=0)
    {
        unsigned long chunkIndex;
        unsigned int frameOffset;

        this->frameToChunk(frame, &chunkIndex, &frameOffset);
        if(0 == this->verifyChunk(chunkIndex))
        {
            int chunkOffset = frame - chunkIndex * DB_SND_BUF_CHUNKSIZE;
        }
        else
            return (SAMPLE) 0;
    }

    void cleanup();
    int loadChunk(unsigned long chunkIndex); // see .cpp file

    void frameToChunk(unsigned long frame, 
            unsigned long *chunkIndex, unsigned int *chunkOffset)
    {
        // nb: we assume frame has been bound/loop-checked by caller
        *chunkIndex = frame / DB_SND_BUF_CHUNKSIZE;
        *chunkOffset = frame - (*chunkIndex*DB_SND_BUF_CHUNKSIZE); 
    }

    int verifyChunk(unsigned long chunkIndex)
    {
        if(this->m_chunkMap[chunkIndex]) // already loaded
            return 0;
        else
            return this->loadChunk(chunkIndex);
    }

    unsigned long calcFrame(long frame, int steps)
    {
        if(!this->m_reverse)
        {
            frame += steps;
            if(frame >= this->m_numFrames)
            {
                if(this->m_loop)
                    frame -= this->m_numFrames;
                else
                    frame = this->m_numFrames-1;
            }
            return (unsigned long) frame;
        }
        else
        {
            frame -= steps;
            if(frame < 0)
            {
                if(this->m_loop)
                    frame += this->m_numFrames;
                else
                    frame = 0;
            }
            return (unsigned long) frame;
        }
    }

    void rateChanged()
    {
        this->m_currentRate = this->m_sampleRatio * this->m_rateMult;
        this->m_reverse = this->m_currentRate < 0.;
        this->m_filterRadius = 1 + std::abs(static_cast<int>(.5*this->m_currentRate));
    }


private:
    float m_chuckSampleRate;

    // sndfile state -------------
    int m_chan;
    int m_numChan;
    unsigned long m_numFrames; // for multi-chan, a frame is comprised of nChan samps
    unsigned long m_numSamps; // == m_numFrames * m_numChan
    unsigned long m_sampleRate; // of the file

    // file-load state ------------
    SAMPLE **m_chunkMap;        // chuck_def
    unsigned long m_numChunks;  // non-zero after ReadHeader
    unsigned long m_framesPerChunk;
    unsigned long m_chunkNum;
    unsigned long m_chunksRead;
    float m_sampleRatio;    // ratio of file sample-rate to chuck samplerate
    void *m_audiofile;

    // playback state ------------
    bool m_loop; 
    float m_rateMult;       // specified by user
    double m_currentFrame;   // fractional in all but rate == 1 cases
                            // use of float means that we're limited to 
                            // framesizes < 2^24 (16.8M). Since we've
                            // gone to the trouble of unsigned long numFrames
                            // we use double for m_currentFrame which
                            // gives framesizes < 2^53

    //
    // m_currentRate combines sampleRatio and rateMult to give us a
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
    float m_currentRate;    
    int m_filterRadius; // int(abs(.5*m_currentRate)) + 1)
    bool m_reverse;
    SAMPLE m_currentSample;
};

#endif