#include "DbBox2D.h"
#include <cassert>

/* Scene construction --------------------------------------------------- */
void
DbBox2D::WorldBegin(t_CKCOMPLEX &gravity)
{
    m_loadWorld = true;
    this->cleanupWorld();
	b2Vec2 g;
	g.Set(gravity.re, gravity.im);
    if(m_debug)
        std::cerr << "New World with gravity " << gravity.re << " " << gravity.im << "\n";
    m_world = new b2World(g);
	m_world->SetContactListener(this);
}

void
DbBox2D::WorldEnd()
{
    m_loadWorld = false;
}

int 
DbBox2D::NewEdge(t_CKCOMPLEX &p1, t_CKCOMPLEX &p2)
{
    int id = m_bodies.size();
    b2BodyDef bd;
    bd.type = b2_staticBody; // XXX
    bd.position.Set((p1.re+p2.re)/2.f, (p1.im+p2.im)/2.f);
    b2Body* body = m_world->CreateBody(&bd);
    body->GetUserData().pointer = id;
    m_bodies.push_back(body);
    // std::cerr << "New Edge " << body->GetUserData().pointer << "\n";

    b2EdgeShape shape;
    b2Vec2 b1(p1.re, p1.im);
    b2Vec2 b2(p2.re, p2.im);
    shape.SetTwoSided(b1, b2);
    #if 0
    else
    {
        float dx = b2.x - b1.x;
        float dy = b2.y - b1.y;
        b2Vec2 b0(b1.x - dx, b1.y - dy), b3(b2.x+dx, b2.y+dy);
        shape.SetOneSided(b0, b1, b2, b3); /* unverified */
    }
    #endif
    body->CreateFixture(&shape, 0.f); /* zero okay since we're static */
    return id;
}

int 
DbBox2D::NewPoint(t_CKCOMPLEX &pos)
{
    int id = m_bodies.size();
    b2BodyDef bd;
    bd.position.Set(pos.re, pos.im);
    b2Body* body = m_world->CreateBody(&bd);
    m_bodies.push_back(body);
    // no fixtures here.
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
    body->GetUserData().pointer = id;
    m_bodies.push_back(body);

    b2CircleShape shape;
    shape.m_p.Set(0.f, 0.f); //position, relative to body position
    shape.m_radius = radius; //radius

    if(m_debug)
    {
        std::cerr << "NewCircle at " 
            << pos.re << ", " << pos.im
            << " r " << radius
            << " d " << density 
            << " id " << body->GetUserData().pointer
            << "\n";
    }
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
    body->GetUserData().pointer = id;
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
DbBox2D::NewRectangle(t_CKCOMPLEX &pos, t_CKCOMPLEX &sz, float angle, 
    float density, BodyType t)
{
    int id = m_bodies.size();
    b2BodyDef bd;
    bd.type = (b2BodyType) t;
    bd.position.Set(pos.re, pos.im);

    b2Body* body = m_world->CreateBody(&bd);
    body->GetUserData().pointer = id;
    m_bodies.push_back(body);

    float hx = sz.re/2.f;
    float hy = sz.im/2.f;
    b2PolygonShape shape;
    shape.SetAsBox(hx, hy, b2Vec2(0, 0), angle);
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
    shape.SetAsBox(hx+hthick, hthick/*hy*/, b2Vec2(0.f, hy+hthick), 0.f);
    body->CreateFixture(&shape, density);
    // bottom
    shape.SetAsBox(hx+hthick, hthick/*hy*/, b2Vec2(0.f, -hy-hthick), 0.f);
    body->CreateFixture(&shape, density);
    // left
    shape.SetAsBox(hthick, hy+hthick/*hy*/, b2Vec2(-hx-hthick, 0.f), 0.f);
    body->CreateFixture(&shape, density);
    // right
    shape.SetAsBox(hthick, hy+hthick/*hy*/, b2Vec2(hx+hthick, 0.f), 0.f);
    body->CreateFixture(&shape, density);

    return id;
}

int 
DbBox2D::NewRevoluteJoint(int bodyA, int bodyB,
        t_CKCOMPLEX &anchorA, t_CKCOMPLEX &anchorB,
        float refAngle, float motorSpeed, float maxMotorTorque)
{
    int jid = -1;
    if(m_world && bodyA < m_bodies.size() && bodyB < m_bodies.size())
    {
        jid = m_joints.size();
        b2Body *a = m_bodies[bodyA];
        b2Body *b = m_bodies[bodyB];

        b2RevoluteJointDef jd;
        jd.bodyA = a;
        jd.bodyB = b;
        jd.localAnchorA.Set(anchorA.re, anchorA.im);
        jd.localAnchorB.Set(anchorB.re, anchorB.im);
        jd.referenceAngle = refAngle;
        jd.motorSpeed = motorSpeed;
        jd.maxMotorTorque = maxMotorTorque;
        jd.enableMotor = motorSpeed > 0.f;

        b2RevoluteJoint *m_joint = (b2RevoluteJoint *) m_world->CreateJoint(&jd);
        m_joints.push_back(m_joint);
    }
    return jid;
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
DbBox2D::GetAngle(int bodyId)
{
    float vel;
    if(m_bodies.size() > bodyId)
    {
        m_worldMutex.lock();
        vel = m_bodies[bodyId]->GetAngle();
        m_worldMutex.unlock();
    }
    else
        vel = 0.f;
    return vel;
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
    #if 0
    std::cerr << "BeginContact"
        << " touching " << contact->IsTouching()
        << " bodyA " << (int) contact->GetFixtureA()->GetBody()->GetUserData().pointer
        << " bodyB " << (int) contact->GetFixtureB()->GetBody()->GetUserData().pointer
        <<"\n";
    #endif
    m_contacts.push_back(contact);
}

void 
DbBox2D::EndContact(b2Contact* contact)
{
#if 0
    std::cerr << "EndContact"
        << " touching " << contact->IsTouching()
        << " bodyA " << (int) contact->GetFixtureA()->GetBody()->GetUserData().pointer
        << " bodyB " << (int) contact->GetFixtureB()->GetBody()->GetUserData().pointer
        <<"\n";
#endif
    m_contacts.push_back(contact);
}

int
DbBox2D::GetContact(int id, int *bodyA, int *bodyB, bool *touching)
{
    if(id < m_contacts.size())
    {
        b2Contact* contact = m_contacts[id];
        *bodyA = (int) contact->GetFixtureA()->GetBody()->GetUserData().pointer;
        *bodyB = (int) contact->GetFixtureB()->GetBody()->GetUserData().pointer;
        assert(*bodyA >= 0 && *bodyB >= 0);
        *touching = contact->IsTouching();
        return 0;
    }
    else
    {
        *bodyA = *bodyB = -1;
        return -1;
    }
}
