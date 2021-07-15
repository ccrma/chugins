#ifndef dbPhasor_h
#define dbPhasor_h

#include <iostream>

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
        wobbleFreq(0.f),
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

    void Tick() 
    {
        this->pos += this->deltaPos;
        if(this->pos > this->stopPos)
        {
            this->pos = this->startPos + (this->pos - this->stopPos);
        }
        else
        if(this->pos < this->startPos)
        {
            this->pos = this->stopPos - (this->startPos - this->pos);
        }
        // XXX: apply wobble
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

    void SetStop(float stop)
    {
        this->stop = stop;
        this->updatePos();
    }

    void SetRate(float rate)
    {
        this->rate = rate;
        this->updatePos();
    }

    void SetWobble(float wobble)
    {
        this->wobble = wobble;
    }

private:

    void updatePos()
    {
        this->startPos = this->start * this->fileTicks;
        this->stopPos = this->stop * this->fileTicks;
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
                << (long) this->stopPos << std::endl;
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

    int debug;
};

#endif