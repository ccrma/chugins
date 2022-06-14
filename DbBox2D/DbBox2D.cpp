#include "DbBox2D.h"

DbBox2D::DbBox2D() :
    m_world(nullptr),
    m_velocityIterations(6),
    m_positionIterations(2),
    m_debug(0),
    m_done(false),
    m_loadWorld(false),
    m_iter(0)
{
    m_mainThreadId = std::this_thread::get_id();
    m_workThread = std::thread(workThreadFunc, this);
}

DbBox2D::~DbBox2D()
{
    // std::cerr << "DbBox2D dtor\n";
    m_done = true;
    if(m_workThread.joinable())
    {
        m_workQueue.Bail();
        m_workThread.join();
    }
}

/* ------------------------------------------------------------------------- */
int
DbBox2D::GetNumContacts()
{ 
    return m_contacts.size(); 
}

int
DbBox2D::GetNumBodies()
{ 
    return m_bodies.size(); 
}

int
DbBox2D::GetNumJoints()
{ 
    return m_joints.size(); 
}

/* ---------------------------------------------------------------------- */
void 
DbBox2D::Step(float timeStep) // invoked in audio thread
{
    std::function<void()> fn = std::bind(&DbBox2D::doStep, this, timeStep);
    m_workQueue.Push(fn);
}

float
DbBox2D::GetAvgSimTime()
{
    if(m_iter ==0) return 0.f;
    return (m_timer.GetElapsed() / m_iter) / std::chrono::seconds(1);
}

void
DbBox2D::doStep(float timeStep)
{
    m_timer.Start();
    std::lock_guard<std::mutex> lock(m_worldMutex);
    if(m_world) 
    {
        m_contacts.clear();
        m_world->Step(timeStep, m_velocityIterations, m_positionIterations);
    }
    Timer::t_Duration dt = m_timer.Stop();
    float timeConsumed = dt / std::chrono::seconds(1);
    if(timeConsumed >= .75f*timeStep)
    {
        static int done = 0;
        if(!done)
        {
            done = 1;
            std::cerr << "DbBox2D WARNING: scene is too heavy for timeStep? " 
                << timeConsumed/timeStep << "\n";
        }
    }
    m_iter++;
}

/*static*/ void
DbBox2D::workThreadFunc(DbBox2D *p)
{
    std::cerr << "Box2D workthread start\n";
    p->m_workThreadId = std::this_thread::get_id();
    while(p->m_workQueue.IsActive())
    {
        auto item = p->m_workQueue.Pop(); // should block when empty
        if(p->m_debug > 1)
        {
            std::cerr << "workThread delegateBegin " 
                     << p->m_workQueue.Size() << "-------------------\n";
        }
        item();  // <------------- work!
        if(p->m_debug > 1)
        {
            std::cerr << "workThread delegateEnd active:" 
                     << p->m_workQueue.IsActive() << "---------------------\n";
        }
    }
    if(p->m_debug)
        std::cerr << "Box2D workThread exiting\n";
}
