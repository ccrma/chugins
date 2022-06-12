#ifndef DbBox2D_h
#define DbBox2D_h

#include <thread>
#include <mutex>
#include <chrono>
#include <functional>
#include <atomic>
#include <iostream>

#include "chuck_oo.h"
#include "box2d/box2d.h"
#include "concurrentQ.h"
#include "timer.h"

// https://box2d.org/documentation/
// http://www.iforce2d.net/b2dtut/constant-speed
// https://piqnt.com/planck.js/

class DbBox2D : public b2ContactListener
{
public:
    DbBox2D();

    // NB: chuck is calling our destructor twice causing badness.
    //  We presume this is because of our unusual attempts to subclass
    //  Chuck_Event.  For now, main.cpp ad
    ~DbBox2D();

    void WorldBegin(t_CKCOMPLEX &gravity);
    void WorldEnd();
    enum BodyType // match b2BodyType
    {
        k_Static=0,
        k_Kinematic, // can move but "always wins", eg platform in game.
        k_Dynamic,
        k_NumBodyTypes
    };
    int NewEdge(t_CKCOMPLEX &p1, t_CKCOMPLEX &p2, bool twoSided);
    int NewCircle(t_CKCOMPLEX &pos, float radius, float density, BodyType t);
    int NewTriangle(t_CKCOMPLEX &p1, t_CKCOMPLEX &p2, t_CKCOMPLEX &p3, float density, BodyType t);
    int NewRectangle(t_CKCOMPLEX &pos, t_CKCOMPLEX &sz, float density, BodyType t);
    int NewRoom(t_CKCOMPLEX &pos, t_CKCOMPLEX &sz, float density, BodyType t);
    // int NewPendulum1()
    // complex NewPendulum2()

    // http://www.iforce2d.net/b2dtut/joints-revolute
    // https://github.com/erincatto/box2d/blob/main/testbed/tests/tumbler.cpp
    int NewRevoluteJoint(int body1, int body2); // anchor is body1 pos
    int NewDistanceJoint(int body1, int body2);
    int NewSpringJoint(int body1, int body2);

    // SetTransform(int i, t_CKCOMPLEX pos, float angle);

    /* can be called while the simulation is running --------------------*/
    // http://www.iforce2d.net/b2dtut/forces
    void ApplyImpulse(int bodyId, t_CKCOMPLEX &impulse); // to center
    void ApplyAngularImpulse(int bodyId, float impulse);

    int SetGravity(t_CKCOMPLEX &g);
    int SetRestitution(int bodyId, float x);
    int SetFriction(int bodyId, float x);
    int SetDensity(int bodyId, float x);

    int GetPosition(int bodyId, t_CKCOMPLEX &pos);
    int GetVelocity(int bodyId, t_CKCOMPLEX &vel);
    float GetAngularVelocity(int bodyId);

    int GetNumContacts(); 
    int GetContact(int id, int *bodyA, int *bodyB, bool *touching);

    int Pause(bool p);

    void Step(float timeStep); // in sec ie: 1./60.
    float GetAvgSimTime(); // in sec
    void Done();

public:
    // contact listener
    void BeginContact(b2Contact* contact) override;
    void EndContact(b2Contact* contact) override;

private:
    b2World *m_world;
    // we currently assert that a body has a single fixture
    // this means collisions (which reference two fixtures)
    // can lead to bodies.
    std::vector<b2Body *> m_bodies; 
    std::vector<b2Joint *> m_joints; 
    std::vector<b2Contact *> m_contacts; 

    int32 m_velocityIterations;
    int32 m_positionIterations;
    void cleanupWorld();

private: // threading
    static void workThreadFunc(DbBox2D *o);
    void doStep(float timeStep); // invoked in work thread
    ConcurrentQ<std::function<void()>> m_workQueue;
    int m_debug;
    std::atomic<bool> m_done;
    std::atomic<bool> m_loadWorld;
    std::thread::id m_mainThreadId, m_workThreadId;
    std::thread m_workThread;
    std::mutex m_worldMutex;
    uint64_t m_iter;
    Timer m_timer;
};

#endif