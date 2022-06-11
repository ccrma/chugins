#include "DbBox2D.h"

DbBox2D::DbBox2D(Chuck_Event *o) :
    m_evtObj(o),
    m_world(nullptr),
    m_done(false),
    m_loadWorld(false),
    m_timeStep(1.f/60.f),
    m_velocityIterations(6),
    m_positionIterations(2)
{
    m_workThread = std::thread(workThreadFunc, this);
}

DbBox2D::~DbBox2D()
{
    // std::cerr << "DbBox2D dtor\n";
    this->Done();
}

/* Scene construction --------------------------------------------------- */
void
DbBox2D::WorldBegin(t_CKCOMPLEX &gravity)
{
    m_loadWorld = true;
    this->cleanupWorld();
}

void
DbBox2D::WorldEnd()
{
    m_loadWorld = false;
}

int 
DbBox2D::NewEdge(t_CKCOMPLEX &p1, t_CKCOMPLEX &p2, bool twoSided)
{
}

int 
DbBox2D::NewCircle(t_CKCOMPLEX &pos, float radius, float density, BodyType t)
{
}

int 
DbBox2D::NewTriangle(t_CKCOMPLEX &p1, t_CKCOMPLEX &p2, t_CKCOMPLEX &p3,     
            float density, BodyType t)
{
}

int 
DbBox2D::NewRectangle(t_CKCOMPLEX &pos, t_CKCOMPLEX &sz, float density, BodyType t)
{
}

int 
DbBox2D::NewRoom(t_CKCOMPLEX &pos, t_CKCOMPLEX &sz, float density, BodyType t)
{
}

void
DbBox2D::cleanupWorld()
{
    m_worldMutex.lock();
    if(m_world)
    {
        delete m_world;
        m_world = nullptr;
    }
    m_bodies.clear();
    m_joints.clear();
    m_worldMutex.unlock();
}

/* ---------------------------------------------------------------------- */
void 
DbBox2D::BeginContact(b2Contact* contact)
{}

void 
DbBox2D::EndContact(b2Contact* contact)
{}

/* ---------------------------------------------------------------------- */
void
DbBox2D::Done()
{
    std::cerr << "Done ---------------------------\n";
    m_done = true;
    // std::this_thread::sleep_for(std::chrono::milliseconds(20));
    // m_workThread.join();
    this->cleanupWorld();
}

/*static*/ void
DbBox2D::workerThreadFunc(DbBox2D *o)
{
    std::cerr << "Box2D workthread start\n";
    o->m_workThreadId = std::this_thread::get_id();
    do
    {
        if(o->m_loadWorld)
            o->loadWorld();
        o->Step();
    } while(o->m_done == false);
    std::cerr << "Box2D workthread done!\n";
}

void 
DbBox2D::Test()
{
    if(m_world)
    {
        m_bodies.clear();
        delete m_world;
    }
    // https://box2d.org/documentation/md__d_1__git_hub_box2d_docs_hello.html
    b2Vec2 gravity(0.0f, -10.0f);
    m_world = new b2World(gravity);
    m_world->SetContactListener(this);

    // creating a ground box.
    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(0.0f, -10.0f);
    b2Body* groundBody = m_world->CreateBody(&groundBodyDef);
    b2PolygonShape groundBox;
    groundBox.SetAsBox(50.0f, 10.0f);
    groundBody->CreateFixture(&groundBox, 0.0f); // massless (ie not dynamic)
    m_bodies.push_back(groundBody);

    // creating a dynamic body
    b2BodyDef bodyDef;
    bodyDef.type = b2_dynamicBody;
    bodyDef.position.Set(0.0f, 4.0f);
    b2Body* body = m_world->CreateBody(&bodyDef);
    b2PolygonShape dynamicBox;
    dynamicBox.SetAsBox(1.0f, 1.0f);
    b2FixtureDef fixtureDef;
    fixtureDef.shape = &dynamicBox;
    fixtureDef.density = 5.0f;
    fixtureDef.friction = 0.1f;
    fixtureDef.restitution = 1.f;
    fixtureDef.restitutionThreshold = 10.f;
    body->CreateFixture(&fixtureDef);
    m_bodies.push_back(body);
}

void 
DbBox2D::Step() // invoked in workThread
{
    if(m_done) return;

    std::this_thread::sleep_for(std::chrono::milliseconds(1000*m_timeStep));
    if(!m_loadWorld)
    {
        m_worldMutex.lock();
        if(m_world) 
        {
            m_world->Step(m_timeStep, m_velocityIterations, m_positionIterations);
            b2Vec2 position = m_bodies[1]->GetPosition();
            float angle = m_bodies[1]->GetAngle();
            fprintf(stderr, "%4.2f %4.2f %4.2f\n", position.x, position.y, angle);
            m_evtObj->broadcast_local(); 
        }
        m_worldMutex.unlock();
    }
    // Following requires fiddle branch of chuck where this method is 
    // declared virtual and thereby resolves a link error.
}