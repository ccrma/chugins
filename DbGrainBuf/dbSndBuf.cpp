#include "dbSndBuf.h"
#include "AudioFile.h"

#include <chuck_errmsg.h>

dbSndBuf::dbSndBuf(float sampleRate)
{
    this->m_chuckSampleRate = sampleRate;
    this->m_chunkMap = nullptr;
    this->m_loop = false;
    this->m_rateMult = 1.f;
    this->m_sndfile = (SNDFILE *) nullptr;
    this->cleanup();
}

dbSndBuf::~dbSndBuf()
{
    this->cleanup();
}

void
dbSndBuf::cleanup()
{
    if(this->m_sndfile != nullptr)
    {
    }
    if(this->m_chunkMap != nullptr)
    {
        for(int i=0;i<this->m_numChunks;i++)
        {
            if(this->m_chunkMap[i])
                delete [] this->m_chunkMap[i];
        }
        delete [] this->m_chunkMap;
        this->m_chunkMap = nullptr;
    }
    this->m_chan = 0;
    this->m_numChan = 0;
    this->m_numFrames = 0;
    this->m_numSamps = 0;
    this->m_sampleRate = 0; // of the file

    this->m_numChunks = 0;
    this->m_framesPerChunk = 0;
    this->m_chunkNum = 0;
    this->m_chunksRead = 0;
    this->m_currentFrame = 0.f;
    this->m_currentRate = 1.f;
    this->m_sampleRatio = 1.f; // ratio of filerate to chuckrate
    this->m_currentSample = (SAMPLE) 0.f;
}

/* ------------------------------------------------------------------------- */
int
dbSndBuf::ReadHeader(char const *filename)
{
    int err = 0;
    this->cleanup();

    SF_INFO info;
    this->m_sndfile = sf_open(filename, SFM_READ, &info);
    err = sf_error(this->m_sndfile);
    if(!err)
    {
        unsigned long size = info.channels * info.frames;

        this->m_numChan = info.channels;
        this->m_numFrames = info.frames;
        this->m_sampleRate = info.samplerate;
        this->m_numSamps = size;

        this->m_numChunks = size / DB_SND_BUF_CHUNKSIZE + 1; // usually fractional, so round up
        this->m_chunkMap = new SAMPLE*[this->m_numChunks];
        memset(this->m_chunkMap, 0, this->m_numChunks * sizeof(SAMPLE *));
        this->m_chunksRead = 0;
        this->m_framesPerChunk = DB_SND_BUF_CHUNKSIZE / this->m_numChan;

        this->m_sampleRatio = this->m_sampleRate / (double) this->m_chuckSampleRate;
        this->rateChanged();
        this->m_currentSample = (SAMPLE) 0;
        this->m_currentFrame = -1;
    }
    else
    {
        CK_FPRINTF_STDERR(
            "[chuck](via dbSndBuf): sndfile error '%li' opening '%s'...\n", 
            err, filename);
        CK_FPRINTF_STDERR("[chuck](via dbSndBuf): (reason: %s)\n", 
            sf_strerror(this->m_sndfile));
        if(this->m_sndfile) 
        {
            sf_close(this->m_sndfile);
            this->m_sndfile = (SNDFILE*) nullptr;
        }
    }
    return err;
}

int
dbSndBuf::loadChunk(unsigned long chunkIndex) // NB: samp = frame*chan
{
    if(this->m_chunkMap[chunkIndex]) return 0;
    int err = 0;
    if(this->m_sndfile)
    {
        SAMPLE *newChunk = new SAMPLE[DB_SND_BUF_CHUNKSIZE];
        this->m_chunkMap[chunkIndex] = newChunk;
        unsigned long numFrames = this->m_framesPerChunk;
        unsigned long startFrame = chunkIndex * this->m_framesPerChunk;
        unsigned long endFrame = startFrame + this->m_framesPerChunk - 1;
        if(endFrame >= this.m_numFrames)
            numFrames = this.m_numFrames - startFrame; // end-of-file condition

        sf_seek(this->m_sndfile, startFrame, SEEK_SET);
        unsigned int n = sf_readf_float(this->m_sndfile, newChunk, numFrames);
        assert(n == numFrames);
        this->m_chunksRead++;
        if(this->m_chunksRead == this->m_numChunks)
        {
            // we've read them all
            sf_close(this->m_sndfile);
            this->m_sndfile = nullptr;
        }
    }
    else
        err = 1;
    return err;
}
