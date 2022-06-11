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
	b2Vec2 g;
	g.Set(gravity.re, gravity.im);
    m_world = new b2World(g);
	m_world->SetContactListener(this);
}

void
DbBox2D::WorldEnd()
{
    m_loadWorld = false;
}

int 
DbBox2D::NewEdge(t_CKCOMPLEX &p1, t_CKCOMPLEX &p2, bool twoSided)
{
    int id = m_bodies.size();
    b2BodyDef bd;
    bd.type = b2_staticBody; // XXX
    bd.position.Set((p1.re+p2.re)/2.f, (p1.im+p2.im)/2.f);
    b2Body* body = m_world->CreateBody(&bd);
    m_bodies.push_back(body);

    b2EdgeShape shape;
    b2Vec2 b1(p1.re, p1.im);
    b2Vec2 b2(p1.re, p2.im);
    if(twoSided)
        shape.SetTwoSided(b1, b2);
    else
    {
        float dx = b2.x - b1.x;
        float dy = b2.y - b1.y;
        b2Vec2 b0(b1.x - dx, b1.y - dy), b3(b2.x+dx, b2.y+dy);
        shape.SetOneSided(b0, b1, b2, b3); /* unverified */
    }
    body->CreateFixture(&shape, 0.f); /* zero okay since we're static */
    return id;
}

int 
DbBox2D::NewCircle(t_CKCOMPLEX &pos, float radius, float density, BodyType t)
{
    int id = m_bodies.size();
    b2BodyDef bd;
    bd.type = (b2BodyType) t;
    bd.position.Set(pos.re, pos.im);
    b2Body* body = m_world->CreateBody(&bd);
    m_bodies.push_back(body);

    b2CircleShape shape;
    shape.m_p.Set(0.f, 0.f); //position, relative to body position
    shape.m_radius = radius; //radius
    body->CreateFixture(&shape, density);
    return id;
}

int 
DbBox2D::NewTriangle(t_CKCOMPLEX &p1, t_CKCOMPLEX &p2, t_CKCOMPLEX &p3,     
            float density, BodyType t)
{
    int id = m_bodies.size();
    b2BodyDef bd;
    bd.type = (b2BodyType) t;
    bd.position.Set((p1.re + p2.re + p3.re)/3.f,
                    (p2.im + p2.im + p3.im)/3.f);
    b2Body* body = m_world->CreateBody(&bd);
    m_bodies.push_back(body);

    b2PolygonShape shape;
    b2Vec2 verts[3];
    verts[0].Set(p1.re, p1.im);
    verts[1].Set(p2.re, p2.im);
    verts[2].Set(p3.re, p3.im);
    shape.Set(verts, 3);
    body->CreateFixture(&shape, density);
    return id;
}

int 
DbBox2D::NewRectangle(t_CKCOMPLEX &pos, t_CKCOMPLEX &sz, float density, BodyType t)
{
    int id = m_bodies.size();
    b2BodyDef bd;
    bd.type = (b2BodyType) t;
    bd.position.Set(pos.re, pos.im);

    b2Body* body = m_world->CreateBody(&bd);
    m_bodies.push_back(body);

    float hx = sz.re/2.f;
    float hy = sz.im/2.f;
    b2PolygonShape shape;
    shape.SetAsBox(hx, hy);
    body->CreateFixture(&shape, density);
    return id;
}

// room has a position and size.  wall thickness is currently set to 1,
// so size should be bigger than that.
int 
DbBox2D::NewRoom(t_CKCOMPLEX &pos, t_CKCOMPLEX &sz, float density, BodyType t)
{
    int id = m_bodies.size();
    b2BodyDef bd;
    bd.type = (b2BodyType) t;
    bd.position.Set(pos.re, pos.im);
    bd.allowSleep = false; // (necessary when static?)
    b2Body *body = m_world->CreateBody(&bd);
    body->GetUserData().pointer = id;
    m_bodies.push_back(body);

    // add four Fixtures to the body, each fixture has-a poly shape.
    b2PolygonShape shape;
    float hthick = 1.0f / 2.f;
    float hx = sz.re/2.f;
    float hy = sz.im/2.f;
    // |--- w ----|
    // /==========\
    // #          #
    // #    c     #
    // #          #
    // \==========/
    // top
    shape.SetAsBox(hx+hthick, hthick/*hy*/, b2Vec2(0.f, hy), 0.f);
    body->CreateFixture(&shape, density);
    // bottom
    shape.SetAsBox(hx+hthick, hthick/*hy*/, b2Vec2(0.f, -hy), 0.f);
    body->CreateFixture(&shape, density);
    // left
    shape.SetAsBox(hthick, hy+hthick/*hy*/, b2Vec2(-hx, 0.f), 0.f);
    body->CreateFixture(&shape, density);
    // right
    shape.SetAsBox(hthick, hy+hthick/*hy*/, b2Vec2(hx, 0.f), 0.f);
    body->CreateFixture(&shape, density);

    return id;
}
void
DbBox2D::ApplyImpulse(int bodyId, t_CKCOMPLEX &impulse) // to center
{
    if(m_bodies.size() > bodyId)
    {
        m_worldMutex.lock();
        m_bodies[bodyId]->ApplyLinearImpulseToCenter(   
            b2Vec2(impulse.re, impulse.im), true/*wake*/);
        m_worldMutex.unlock();
    }
}

void
DbBox2D::ApplyAngularImpulse(int bodyId, float impulse)
{
    if(m_bodies.size() > bodyId)
    {
        m_worldMutex.lock();
        m_bodies[bodyId]->ApplyAngularImpulse(impulse, true/*wake*/);
        m_worldMutex.unlock();
    }
}

int
DbBox2D::SetGravity(t_CKCOMPLEX &g)
{
    m_worldMutex.lock();
    if(m_world)
        m_world->SetGravity(b2Vec2(g.re, g.im));
    m_worldMutex.unlock();
    return 0;
}

int
DbBox2D::SetRestitution(int bodyId, float x)
{
    int err=0;
    if(m_bodies.size() > bodyId)
    {
        m_worldMutex.lock();
        b2Fixture *f = m_bodies[bodyId]->GetFixtureList();
        while(f)
        {
            f->SetRestitution(x); // usually a single fixture
            f = f->GetNext();
        }
        m_worldMutex.unlock();
    }
    else
        err = 1;
    return err;
}

int
DbBox2D::SetFriction(int bodyId, float x)
{
    int err=0;
    if(m_bodies.size() > bodyId)
    {
        m_worldMutex.lock();
        b2Fixture *f = m_bodies[bodyId]->GetFixtureList();
        while(f)
        {
            f->SetFriction(x); // usually a single fixture
            f = f->GetNext();
        }
        m_worldMutex.unlock();
    }
    else
        err = 1;
    return err;
}

int
DbBox2D::SetDensity(int bodyId, float x)
{
    int err=0;
    if(m_bodies.size() > bodyId)
    {
        m_worldMutex.lock();
        b2Fixture *f = m_bodies[bodyId]->GetFixtureList();
        while(f)
        {
            f->SetDensity(x); // usually a single fixture
            f = f->GetNext();
        }
        m_worldMutex.unlock();
    }
    else
        err = 1;
    return err;
}

int 
DbBox2D::GetPosition(int bodyId, t_CKCOMPLEX &pos)
{
    int err=0;
    if(m_bodies.size() > bodyId)
    {
        m_worldMutex.lock();
        const b2Vec2 &bp = m_bodies[bodyId]->GetPosition();
        pos.re = bp.x;
        pos.im = bp.y;
        m_worldMutex.unlock();
    }
    else
        err = 1;
    return err;
}

int 
DbBox2D::GetVelocity(int bodyId, t_CKCOMPLEX &vel)
{
    int err=0;
    if(m_bodies.size() > bodyId)
    {
        m_worldMutex.lock();
        const b2Vec2 &bp = m_bodies[bodyId]->GetLinearVelocity();
        vel.re = bp.x;
        vel.im = bp.y;
        m_worldMutex.unlock();
    }
    else
        err = 1;
    return err;
}

float 
DbBox2D::GetAngularVelocity(int bodyId)
{
    float vel;
    if(m_bodies.size() > bodyId)
    {
        m_worldMutex.lock();
        vel = m_bodies[bodyId]->GetAngularVelocity();
        m_worldMutex.unlock();
    }
    else
        vel = 0.f;
    return vel;
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
{
    m_contacts.push_back(contact);
}

void 
DbBox2D::EndContact(b2Contact* contact)
{
    m_contacts.push_back(contact);
}

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
DbBox2D::workThreadFunc(DbBox2D *o)
{
    std::cerr << "Box2D workthread start\n";
    o->m_workThreadId = std::this_thread::get_id();
    do
    {
        o->Step();
    } while(o->m_done == false);
    std::cerr << "Box2D workthread done!\n";
}

void 
DbBox2D::Step() // invoked in workThread
{
    if(m_done) return;

    std::this_thread::sleep_for(std::chrono::milliseconds(int(1000*m_timeStep)));
    if(!m_loadWorld)
    {
        m_worldMutex.lock();
        if(m_world) 
        {
            m_world->Step(m_timeStep, m_velocityIterations, m_positionIterations);
            m_contactStates.clear();
            // copy-out our contacts...
            for(int i=0;i<m_contacts.size();i++)
                m_contactStates.push_back(ContactState(m_contacts[i]));
            m_contacts.clear();
            m_evtObj->broadcast_local(); 
        }
        m_worldMutex.unlock();
    }
    // Following requires fiddle branch of chuck where this method is 
    // declared virtual and thereby resolves a link error.
}