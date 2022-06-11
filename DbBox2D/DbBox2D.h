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

// https://box2d.org/documentation/
// http://www.iforce2d.net/b2dtut/constant-speed
// https://piqnt.com/planck.js/

class DbBox2D : public b2ContactListener
{
public:
    DbBox2D(Chuck_Event *o);

    // NB: chuck is calling our destructor twice causing badness.
    //  We presume this is because of our unusual attempts to subclass
    //  Chuck_Event.  For now, main.cpp ad
    ~DbBox2D();

    void WorldBegin(t_CKCOMPLEX &gravity);
    void WorldEnd();
    enum BodyType
    {
        k_Static,
        k_Dynamic,
        k_Kinematic // can move but "always wins", eg platform in game.
    };
    int NewEdge(t_CKCOMPLEX &p1, t_CKCOMPLEX &p2, bool twoSided);
    int NewCircle(t_CKCOMPLEX &pos, float radius, float density, BodyType t);
    int NewTriangle(t_CKCOMPLEX &p1, t_CKCOMPLEX &p2, t_CKCOMPLEX &p3, float density, BodyType t);
    int NewRectangle(t_CKCOMPLEX &pos, t_CKCOMPLEX &sz, float density, BodyType t);
    int NewRoom(t_CKCOMPLEX &pos, t_CKCOMPLEX &sz, float density, BodyType t);

    // Density, Restitution, Friction properties

    // int NewPendulum1()
    // complex NewPendulum2()

    // http://www.iforce2d.net/b2dtut/joints-revolute
    // https://github.com/erincatto/box2d/blob/main/testbed/tests/tumbler.cpp
    int NewRevoluteJoint(JointType t, int body1, int body2); // anchor is body1 pos
    int NewDistanceJoint(JointType t, int body1, int body2);
    int NewSpringJoint(JointType t, int body1, int body2);

    // SetTransform(int i, t_CKCOMPLEX pos, float angle);
    // SetLinearVelocity(int i, t_CKCOMPLEX vel);
    // SetAngularVelocity(int i, float radsPerSec);
    // vs
    // ApplyLinearForce/Impulse
    // ApplyAngularForce/Impulse

    void Done();

    // contact listener
    void BeginContact(b2Contact* contact) override;
    void EndContact(b2Contact* contact) override;

private:
    static void workThreadFunc(DbBox2D *o);

private:
    b2World *m_world;
    // we currently assert that a body has a single fixture
    // this means collisions (which reference two fixtures)
    // can lead to bodies.
    std::vector<b2Body *> m_bodies; 
    std::vector<b2Joint *> m_joints; 

    float m_timeStep;
    int32 m_velocityIterations;
    int32 m_positionIterations;
    void cleanupWorld();

private:
    Chuck_Event *m_evtObj;
    std::atomic<bool> m_done;
    std::atomic<bool> m_loadWorld;
    std::string m_worldFile;
    std::thread::id m_mainThreadId, m_workThreadId;
    std::thread m_workThread;
    std::mutex m_worldMutex;
};

#endif