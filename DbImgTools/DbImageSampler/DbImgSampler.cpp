/* see comments in README.md */
#include "DbImageSampler.h"

#include <iostream>
#include <functional>
#include <cassert>
#include <string>

DbImageSampler::DbImageSampler() :
    m_image(nullptr),
    m_verbosity(0)
{
    m_mainThreadId = std::this_thread::get_id();
    m_loadThread = std::thread(loadThreadFunc, this);
}

DbImageSampler::~DbImageSampler()
{ 
    if(m_loadThread.joinable())
    {
        m_loadQueue.Bail();
        m_loadThread.join();
    }
    if(m_image)
        delete m_image;
}

void
DbImageSampler::Load(char const *filename)
{
    std::string nm(filename);
    std::function<void()> fn = std::bind(&DbImageSampler::loadImage, 
                                        this, nm/*pass by value*/);
    m_loadQueue.Push(fn);
}

int
DbImageSampler::GetSample(float x, float y, float *r, float *g, float *b, float *a)
{
    // scope for access to image
    std::lock_guard<std::mutex> lock(m_loadImageLock);
    if(m_image)
        return m_image->GetSample(x, y, r, g, b, a);
    else
    {
        *r = *g = *b = *a = 0.f;
        return -1;
    }
}

/* We anticipate multiple edits arriving in clumps and want
 * to limit the reload-image frequency.  If we haven't computed
 * "for a while", we schedule a updateRequest for later. Otherwise
 * the outstanding request will still be in play and it's serviced
 * during Tick.
 */
void
DbImageSampler::makeDirty()
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
DbImageSampler::checkDeferredUpdate()
{
    if(m_scheduledUpdate == m_zeroInstant)
        return;

    auto now = std::chrono::steady_clock::now();
    if(now >= m_scheduledUpdate)
    {
        // std::cerr << "Applying scheduled update\n";
        if(m_imgfilePath.size() > 0)
            this->Load(m_imgfilePath.c_str());
        m_scheduledUpdate = m_zeroInstant;
    }
}

void
DbImageSampler::loadImage(std::string nm) // invoked in loaderThread
{
    assert(std::this_thread::get_id() == m_loadThreadId);
    Image *newImg = new Image();
    int err = newImg->LoadFile(nm.c_str());
    if(!err)
        this->setImage(newImg, nm);
    else
        std::cerr << "DbImageSampler problem opening file " << nm << "\n";
}

// happens after loading
void
DbImageSampler::setImage(Image *i, std::string &nm)
{
    std::lock_guard<std::mutex> lock(m_loadImageLock);
    if(m_image)
        delete m_image;

    m_image = i;
    m_imgfilePath = nm;
    if(m_verbosity)
    {
        std::cerr << "DbImageSampler '" <<  m_image->GetName() 
            << "' ready (h:"
            << i->GetHeight() << " w: " << i->GetWidth() << "\n";
    }
}

/* static */ void
DbImageSampler::loadThreadFunc(DbImageSampler *p)
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