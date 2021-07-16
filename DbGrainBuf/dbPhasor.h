#ifndef dbPhasor_h
#define dbPhasor_h

#include <iostream>
#include "dbRand.h"

/* Phasor produces values between [0, 1] representing the 
 * sndbuf location pointer. When rate is 1, the expectation 
 * is that, when sampled, Phasor returns a phase that matches
 * the natural phase for the sndbuf file (which is a function 
 * of its duration).  For this reason Phasor can't operate without
 * knowledge of the current file duration.
 */
class dbPhasor
{
public:
    dbPhasor(float sr) :
        sampleRate(sr),

        fileTicks(0), // only known after file is provided
        pos(0.),
        startPos(0.),
        stopPos(0.),
        deltaPos(0.),

        start(0.f), // pct of file
        stop(1.f),  // pct of file
        rate(1.f),
        wobble(0.f),
        wobbleFreq(1.f),
        nextWobbleUpdate(0),
        wobbleAmt(0.f),
        debug(0)
    {}
    ~dbPhasor() {};

    // we want phasor to go from min to max in seconds (*rate)
    // samples/sec * sec = samples (in song/phasor), so phaseStep is range / samples
    void SetFileDur(double secs)
    {
        this->fileTicks = secs * this->sampleRate;
        this->updatePos();
    }

    void Debug(int d)
    {
        this->debug = d;
    }

    void Tick(long totalTicks) 
    {
        if(this->startPos > this->stopPos)
        {
            std::cout << "DbGrainBuf phasorStop must be >= phasorStart\n";
            this->stopPos = this->startPos;
        }
        this->pos += this->deltaPos;
        if(this->wobble != 0 && totalTicks > this->nextWobbleUpdate)
        {
            // wobble is a modulation of rate
            this->nextWobbleUpdate = totalTicks + 
                                this->sampleRate/this->wobbleFreq;
            this->wobbleAmt = this->wobble * rand32Pct() * this->rate;

        }
        if(this->stopPos != this->startPos)
        {
            if(this->wobble != 0)
                this->pos += this->wobbleAmt;
            if(this->pos > this->stopPos)
                this->pos = this->startPos + (this->pos - this->stopPos);
            else
            if(this->pos < this->startPos)
                this->pos = this->stopPos - (this->startPos - this->pos);
        }
        else
        {
            // wobble around fixed position
            this->pos = this->startPos + this->wobbleAmt;
        }
    }

    /**
     @returns the current pos value as a sample index. Double cuz
       we can return a inter-sample location. Also: we need more than
       23 bits of file-offset that float provides.
     */
    double Sample()
    {
        if(this->debug)
        {
            std::cout << "Phasor " << this->pos << std::endl;
        }
        return this->pos;
    }

    void SetStart(float start)
    {
        this->start = start;
        this->updatePos();
    }

    float GetStart()
    {
        return this->start;
    }

    void SetStop(float stop)
    {
        this->stop = stop;
        this->updatePos();
    }

    float GetStop()
    {
        return this->stop;
    }

    void SetRate(float rate)
    {
        this->rate = rate;
        this->updatePos();
    }

    void SetWobble(float wobble)
    {
        this->wobble = wobble;
        this->nextWobbleUpdate = 0;
    }

    void SetWobbleFreq(float wfreq)
    {
        this->wobbleFreq = wfreq;
        this->nextWobbleUpdate = 0;
    }

private:

    void updatePos()
    {
        this->startPos = static_cast<double>(this->start) * this->fileTicks;
        this->stopPos = static_cast<double>(this->stop) * this->fileTicks;
        this->pos = this->startPos;
        if(this->startPos == this->stopPos)
            this->deltaPos = 0;
        else
            this->deltaPos = this->rate;
        if(this->stop < this->start)
        {
            /* (happens legitimately when changing bounds)
            std::cout << "Phasor has invalid bounds: " << this->start <<
                " >= " << this->stop << std::endl;
            */
        }
        if(this->debug)
        {
            std::cout << "Phasor init: " << (long) this->startPos << ", "
                << (long) this->stopPos << ", delta: " << this->deltaPos 
                << std::endl;
        }

    }

    float sampleRate; // samples/sec (chuck sample rate)

    // we maintain state in chuck samplecoords relative to file
    long fileTicks; 
    double pos;
    double startPos;
    double stopPos;
    double deltaPos; // need to handle negative delta

    // user expresses phase as pct of file. 
    // we convert to ticks when we know the file size.
    float start; 
    float stop;
    float rate;
    float wobble;
    float wobbleFreq; 
    float wobbleAmt;
    long nextWobbleUpdate;

    int debug;
};

#endif