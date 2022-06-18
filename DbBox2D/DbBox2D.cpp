#include "DbBox2D.h"

DbBox2D::DbBox2D() :
    m_world(nullptr),
    m_velocityIterations(8), // recommended here: https://box2d.org/documentation/md__d_1__git_hub_box2d_docs_hello.html
    m_positionIterations(3),
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
int 
DbBox2D::GetNumBodyShapes(int bodyId)
{
    if(bodyId < m_bodies.size())
    {
        int cnt = 0;
        b2Body *body = m_bodies[bodyId];
        b2Fixture *fixture = body->GetFixtureList(); // usually a single fixture
        while(fixture)
        {
            cnt++;
            fixture = fixture->GetNext();
        }
        return cnt;
    }
    else
        return -1;
}

int 
DbBox2D::GetBodyShapeType(int bodyId, int shapeIndex)
{
    if(bodyId < m_bodies.size())
    {
        b2Body *body = m_bodies[bodyId];
        b2Fixture *fixture = body->GetFixtureList(); // usually a single fixture
        int cnt = 0;
        while(fixture)
        {
            if(shapeIndex == cnt)
                return (int) fixture->GetType();
            cnt++;        
            fixture = fixture->GetNext();

        }
        return -1;
    }
    else
        return -1;
}

float 
DbBox2D::GetCircleRadius(int bodyId, int shapeIndex)
{
    if(bodyId < m_bodies.size())
    {
        b2Body *body = m_bodies[bodyId];
        b2Fixture *fixture = body->GetFixtureList(); // usually a single fixture
        int cnt = 0;
        while(fixture)
        {
            if(shapeIndex == cnt)
                return ((b2CircleShape *) fixture->GetShape())->m_radius;

            cnt++;        
            fixture = fixture->GetNext();
        }
        return 0.f;
    }
    else
        return 0.f;
}

// points are arrays of complex.  each complex is a pair of doubles.
// so item size is 16 bytes, ergo Chuck_Array16.
// An edge is just a line with two points.
int 
DbBox2D::GetEdgePoints(int bodyId, int shapeIndex, std::vector<t_CKCOMPLEX> &pts)
{ 
    pts.clear();
    if(bodyId < m_bodies.size())
    {
        b2Body *body = m_bodies[bodyId];
        b2Fixture *fixture = body->GetFixtureList(); // usually a single fixture
        int cnt = 0;
        while(fixture)
        {
            if(shapeIndex == cnt)
            {
                b2EdgeShape *s = (b2EdgeShape *) fixture->GetShape();
                t_CKCOMPLEX c;
                c.re = s->m_vertex1.x;
                c.im = s->m_vertex1.y;
                pts.push_back(c);
                c.re = s->m_vertex2.x;
                c.im = s->m_vertex2.y;
                pts.push_back(c);
                return 0; // success
            }
            cnt++;        
            fixture = fixture->GetNext();
        }
    }
    return -1;
}

int 
DbBox2D::GetPolygonPoints(int bodyId, int shapeIndex, std::vector<t_CKCOMPLEX> &pts)
{
    pts.clear();
    if(bodyId < m_bodies.size())
    {
        b2Body *body = m_bodies[bodyId];
        b2Fixture *fixture = body->GetFixtureList(); // usually a single fixture
        int cnt = 0;
        while(fixture)
        {
            if(shapeIndex == cnt)
            {
                b2PolygonShape *s = (b2PolygonShape *) fixture->GetShape();
                t_CKCOMPLEX c;
                for(int i=0;i<s->m_count;i++)
                {
                    c.re = s->m_vertices[i].x;
                    c.im = s->m_vertices[i].y;
                    pts.push_back(c);
                }
                return 0; // success
            }
            cnt++;        
            fixture = fixture->GetNext();
        }
    }
    return -1;
}

int 
DbBox2D::GetChainPoints(int bodyId, int shapeIndex, std::vector<t_CKCOMPLEX> &pts)
{
    pts.clear();
    if(bodyId < m_bodies.size())
    {
        b2Body *body = m_bodies[bodyId];
        b2Fixture *fixture = body->GetFixtureList(); // usually a single fixture
        int cnt = 0;
        while(fixture)
        {
            if(shapeIndex == cnt)
            {
                b2ChainShape *s = (b2ChainShape *) fixture->GetShape();
                t_CKCOMPLEX c;
                for(int i=0;i<s->m_count;i++)
                {
                    c.re = s->m_vertices[i].x;
                    c.im = s->m_vertices[i].y;
                    pts.push_back(c);
                }
                return 0; // success
            }
            cnt++;        
            fixture = fixture->GetNext();
        }
    }
    return -1;
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
