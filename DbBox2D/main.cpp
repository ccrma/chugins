/*----------------------------------------------------------------------------
 See comments in DbBox2D.h

-----------------------------------------------------------------------------*/
#include "DbBox2D.h"

#include "chuck_dl.h"
#include "chuck_def.h"


CK_DLL_CTOR( dbb2d_ctor );
CK_DLL_DTOR( dbb2d_dtor );
CK_DLL_MFUN( dbb2d_worldBegin );
CK_DLL_MFUN( dbb2d_newPoint );
CK_DLL_MFUN( dbb2d_newEdge );
CK_DLL_MFUN( dbb2d_newCircle );
CK_DLL_MFUN( dbb2d_newTriangle );
CK_DLL_MFUN( dbb2d_newRectangle );
CK_DLL_MFUN( dbb2d_newRoom );
CK_DLL_MFUN( dbb2d_newRevoluteJoint );
CK_DLL_MFUN( dbb2d_worldEnd );
CK_DLL_MFUN( dbb2d_getNumBodies );

CK_DLL_MFUN( dbb2d_step );
CK_DLL_MFUN( dbb2d_getAvgSimTime );

CK_DLL_MFUN( dbb2d_getPosition );
CK_DLL_MFUN( dbb2d_getVelocity );
CK_DLL_MFUN( dbb2d_getAngle );
CK_DLL_MFUN( dbb2d_getAngularVelocity );
CK_DLL_MFUN( dbb2d_getNumContacts );
CK_DLL_MFUN( dbb2d_getContact );
CK_DLL_MFUN( dbb2d_setGravity );
CK_DLL_MFUN( dbb2d_setFriction );
CK_DLL_MFUN( dbb2d_setDensity );
CK_DLL_MFUN( dbb2d_setRestitution );
CK_DLL_MFUN( dbb2d_applyImpulse );
CK_DLL_MFUN( dbb2d_applyAngularImpulse );

t_CKINT dbb2d_data_offset = 0;

/* enums */
t_CKINT dbb2d_staticType_offset = 0;
t_CKINT dbb2d_kinematicType_offset = 0;
t_CKINT dbb2d_dynamicType_offset = 0;
t_CKINT dbb2d_degToRad_offset = 0;
t_CKINT dbb2d_radToDeg_offset = 0;

/* ----------------------------------------------------------- */
CK_DLL_QUERY(DbBox2D)
{
    QUERY->setname(QUERY, "DbBox2D");
    QUERY->begin_class(QUERY, "DbBox2D", "Object"); // we're not a ugen!

    QUERY->add_ctor(QUERY, dbb2d_ctor);
    QUERY->add_dtor(QUERY, dbb2d_dtor);

    QUERY->add_mfun(QUERY, dbb2d_worldBegin, "void", "worldBegin");
    QUERY->add_arg(QUERY, "complex", "gravity");

    QUERY->add_mfun(QUERY, dbb2d_worldEnd, "void", "worldEnd");
    QUERY->add_mfun(QUERY, dbb2d_getNumBodies, "int", "getNumBodies");

    QUERY->add_mfun(QUERY, dbb2d_newEdge, "int", "newEdge");
    QUERY->add_arg(QUERY, "complex", "p1");
    QUERY->add_arg(QUERY, "complex", "p2");

    QUERY->add_mfun(QUERY, dbb2d_newPoint, "int", "newPoint");
    QUERY->add_arg(QUERY, "complex", "position");

    QUERY->add_mfun(QUERY, dbb2d_newCircle, "int", "newCircle");
    QUERY->add_arg(QUERY, "complex", "position");
    QUERY->add_arg(QUERY, "float", "radius"); 
    QUERY->add_arg(QUERY, "float", "density"); 
    QUERY->add_arg(QUERY, "int", "bodyType");

    QUERY->add_mfun(QUERY, dbb2d_newTriangle, "int", "newTriangle");
    QUERY->add_arg(QUERY, "complex", "p1");
    QUERY->add_arg(QUERY, "complex", "p2");
    QUERY->add_arg(QUERY, "complex", "p3");
    QUERY->add_arg(QUERY, "float", "density"); 
    QUERY->add_arg(QUERY, "int", "bodyType");

    QUERY->add_mfun(QUERY, dbb2d_newRectangle, "int", "newRectangle");
    QUERY->add_arg(QUERY, "complex", "position");
    QUERY->add_arg(QUERY, "complex", "size");
    QUERY->add_arg(QUERY, "float", "angle");
    QUERY->add_arg(QUERY, "float", "density"); 
    QUERY->add_arg(QUERY, "int", "bodyType");

    QUERY->add_mfun(QUERY, dbb2d_newRoom, "int", "newRoom");
    QUERY->add_arg(QUERY, "complex", "position");
    QUERY->add_arg(QUERY, "complex", "size");
    QUERY->add_arg(QUERY, "float", "density");
    QUERY->add_arg(QUERY, "int", "bodyType");

    QUERY->add_mfun(QUERY, dbb2d_newRevoluteJoint, "int", "newRevoluteJoint");
    QUERY->add_arg(QUERY, "int", "bodyA");
    QUERY->add_arg(QUERY, "int", "bodyB");
    QUERY->add_arg(QUERY, "complex", "localAnchorA");
    QUERY->add_arg(QUERY, "complex", "localAnchorB");
    QUERY->add_arg(QUERY, "float", "refAngle");
    QUERY->add_arg(QUERY, "float", "motorSpeed");
    QUERY->add_arg(QUERY, "float", "maxMotorTorque");

    QUERY->add_mfun(QUERY, dbb2d_step, "void", "step");
    QUERY->add_arg(QUERY, "dur", "stepSize");

    QUERY->add_mfun(QUERY, dbb2d_getAvgSimTime, "dur", "getAvgSimTime");

    QUERY->add_mfun(QUERY, dbb2d_getPosition, "complex", "getPosition");
    QUERY->add_arg(QUERY, "int", "bodyId");

    QUERY->add_mfun(QUERY, dbb2d_getVelocity, "complex", "getVelocity");
    QUERY->add_arg(QUERY, "int", "bodyId");

    QUERY->add_mfun(QUERY, dbb2d_getAngle, "float", "getAngle");
    QUERY->add_arg(QUERY, "int", "bodyId");

    QUERY->add_mfun(QUERY, dbb2d_getAngularVelocity, "float", "getAngularVelocity");
    QUERY->add_arg(QUERY, "int", "bodyId");

    QUERY->add_mfun(QUERY, dbb2d_getNumContacts, "int", "getNumContacts");

    QUERY->add_mfun(QUERY, dbb2d_getContact, "vec3", "getContact"); // bodyA, bodyB, touching
    QUERY->add_arg(QUERY, "int", "bodyId");

    QUERY->add_mfun(QUERY, dbb2d_setGravity, "void", "setGravity");
    QUERY->add_arg(QUERY, "complex", "gravity");

    QUERY->add_mfun(QUERY, dbb2d_setFriction, "void", "setFriction" );
    QUERY->add_arg(QUERY, "int", "bodyId");
    QUERY->add_arg(QUERY, "float", "friction");

    QUERY->add_mfun(QUERY, dbb2d_setDensity, "void", "setDensity" );
    QUERY->add_arg(QUERY, "int", "bodyId");
    QUERY->add_arg(QUERY, "float", "density");

    QUERY->add_mfun(QUERY, dbb2d_setRestitution , "void", "setRestitution");
    QUERY->add_arg(QUERY, "int", "bodyId");
    QUERY->add_arg(QUERY, "float", "restitution");

    QUERY->add_mfun(QUERY, dbb2d_applyImpulse, "void", "applyImpulse" );
    QUERY->add_arg(QUERY, "int", "bodyId");
    QUERY->add_arg(QUERY, "complex", "impulse");

    QUERY->add_mfun(QUERY, dbb2d_applyAngularImpulse, "void", "applyAngularImpulse" );
    QUERY->add_arg(QUERY, "int", "bodyId");
    QUERY->add_arg(QUERY, "float", "angularImpulse");

    dbb2d_staticType_offset = QUERY->add_mvar(QUERY, "int", "staticType", true/*constant*/);
    dbb2d_kinematicType_offset = QUERY->add_mvar(QUERY, "int", "kinematicType", true/*constant*/);
    dbb2d_dynamicType_offset = QUERY->add_mvar(QUERY, "int", "dynamicType", true/*constant*/);
    dbb2d_degToRad_offset = QUERY->add_mvar(QUERY, "float", "degToRad", true/*constant*/);
    dbb2d_radToDeg_offset = QUERY->add_mvar(QUERY, "float", "radToDeg", true/*constant*/);

    dbb2d_data_offset = QUERY->add_mvar(QUERY, "int", "@dbb2d_data", false);
    QUERY->end_class(QUERY);
    return TRUE;
}

CK_DLL_CTOR(dbb2d_ctor)
{
    DbBox2D *c = new DbBox2D();
    OBJ_MEMBER_INT(SELF, dbb2d_data_offset) = (t_CKINT) c;
    OBJ_MEMBER_INT(SELF, dbb2d_staticType_offset) = 0;
    OBJ_MEMBER_INT(SELF, dbb2d_kinematicType_offset) = 1;
    OBJ_MEMBER_INT(SELF, dbb2d_dynamicType_offset) = 2;
    OBJ_MEMBER_FLOAT(SELF, dbb2d_degToRad_offset) =  ONE_PI/ 180.f;
    OBJ_MEMBER_FLOAT(SELF, dbb2d_radToDeg_offset) = 180.f / ONE_PI;
}

CK_DLL_DTOR(dbb2d_dtor)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbb2d_data_offset);
    if(c)
    {
        delete c;
        OBJ_MEMBER_INT(SELF, dbb2d_data_offset) = 0;
    }
}

CK_DLL_MFUN(dbb2d_worldBegin)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbb2d_data_offset);
    t_CKCOMPLEX gravity = GET_NEXT_COMPLEX(ARGS);
    c->WorldBegin(gravity); 
}

CK_DLL_MFUN(dbb2d_worldEnd)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbb2d_data_offset);
    c->WorldEnd();
}

CK_DLL_MFUN(dbb2d_getNumBodies)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbb2d_data_offset);
    RETURN->v_int = c->GetNumBodies();
}

CK_DLL_MFUN(dbb2d_newEdge)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbb2d_data_offset);
    t_CKCOMPLEX p1 = GET_NEXT_COMPLEX(ARGS);
    t_CKCOMPLEX p2 = GET_NEXT_COMPLEX(ARGS);
    RETURN->v_int = c->NewEdge(p1, p2);
}

CK_DLL_MFUN(dbb2d_newPoint)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbb2d_data_offset);
    t_CKCOMPLEX pos = GET_NEXT_COMPLEX(ARGS);
    RETURN->v_int = c->NewPoint(pos);
}

CK_DLL_MFUN(dbb2d_newCircle)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbb2d_data_offset);
    t_CKCOMPLEX pos = GET_NEXT_COMPLEX(ARGS);
    float radius = GET_NEXT_FLOAT(ARGS);
    float density = GET_NEXT_FLOAT(ARGS);
    int bt = GET_NEXT_INT(ARGS);
    DbBox2D::BodyType t = DbBox2D::k_Static;
    if(bt >= 0 && bt < DbBox2D::k_NumBodyTypes)
        t = (DbBox2D::BodyType) bt;
    RETURN->v_int = c->NewCircle(pos, radius, density, t);
}

CK_DLL_MFUN(dbb2d_newTriangle)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbb2d_data_offset);
    t_CKCOMPLEX p1 = GET_NEXT_COMPLEX(ARGS);
    t_CKCOMPLEX p2 = GET_NEXT_COMPLEX(ARGS);
    t_CKCOMPLEX p3 = GET_NEXT_COMPLEX(ARGS);
    float density = GET_NEXT_FLOAT(ARGS);
    int bt = GET_NEXT_INT(ARGS);
    DbBox2D::BodyType t = DbBox2D::k_Static;
    if(bt >= 0 && bt < DbBox2D::k_NumBodyTypes)
        t = (DbBox2D::BodyType) bt;
    RETURN->v_int = c->NewTriangle(p1, p2, p3, density, t);
}

CK_DLL_MFUN(dbb2d_newRectangle)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbb2d_data_offset);
    t_CKCOMPLEX pos = GET_NEXT_COMPLEX(ARGS);
    t_CKCOMPLEX sz = GET_NEXT_COMPLEX(ARGS);
    float angle = GET_NEXT_FLOAT(ARGS);
    float density = GET_NEXT_FLOAT(ARGS);
    int bt = GET_NEXT_INT(ARGS);
    DbBox2D::BodyType t = DbBox2D::k_Static;
    if(bt >= 0 && bt < DbBox2D::k_NumBodyTypes)
        t = (DbBox2D::BodyType) bt;
    RETURN->v_int = c->NewRectangle(pos, sz, angle, density, t);
}

CK_DLL_MFUN(dbb2d_newRoom)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbb2d_data_offset);
    t_CKCOMPLEX pos = GET_NEXT_COMPLEX(ARGS);
    t_CKCOMPLEX sz = GET_NEXT_COMPLEX(ARGS);
    float density = GET_NEXT_FLOAT(ARGS);
    int bt = GET_NEXT_INT(ARGS);
    DbBox2D::BodyType t = DbBox2D::k_Static;
    if(bt >= 0 && bt < DbBox2D::k_NumBodyTypes)
        t = (DbBox2D::BodyType) bt;
    RETURN->v_int = c->NewRoom(pos, sz, density, t);
}

CK_DLL_MFUN(dbb2d_newRevoluteJoint)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbb2d_data_offset);
    int bodyA = GET_NEXT_INT(ARGS);
    int bodyB = GET_NEXT_INT(ARGS);
    t_CKCOMPLEX anchorA = GET_NEXT_COMPLEX(ARGS);
    t_CKCOMPLEX anchorB = GET_NEXT_COMPLEX(ARGS);
    float motorSpeed = GET_NEXT_FLOAT(ARGS);
    float refAngle = GET_NEXT_FLOAT(ARGS);
    float maxMotorTorque = GET_NEXT_FLOAT(ARGS);
    RETURN->v_int = c->NewRevoluteJoint(bodyA, bodyB, anchorA, anchorB,
        motorSpeed, refAngle, maxMotorTorque);
}

CK_DLL_MFUN(dbb2d_step)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbb2d_data_offset);
    float stepSize = GET_NEXT_DUR(ARGS) / API->vm->get_srate(API, SHRED);
    c->Step(stepSize); // async, void
}

CK_DLL_MFUN(dbb2d_getAvgSimTime)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbb2d_data_offset);
    RETURN->v_dur = API->vm->get_srate(API, SHRED) * c->GetAvgSimTime();
}

CK_DLL_MFUN(dbb2d_getPosition)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbb2d_data_offset);
    int bodyId = GET_NEXT_INT(ARGS);
    c->GetPosition(bodyId, RETURN->v_complex);
}

CK_DLL_MFUN(dbb2d_getVelocity)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbb2d_data_offset);
    int bodyId = GET_NEXT_INT(ARGS);
    c->GetVelocity(bodyId, RETURN->v_complex);
}

CK_DLL_MFUN(dbb2d_getAngle)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbb2d_data_offset);
    int bodyId = GET_NEXT_INT(ARGS);
    RETURN->v_float = c->GetAngle(bodyId);
}

CK_DLL_MFUN(dbb2d_getAngularVelocity)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbb2d_data_offset);
    int bodyId = GET_NEXT_INT(ARGS);
    RETURN->v_float = c->GetAngularVelocity(bodyId);
}

CK_DLL_MFUN(dbb2d_getNumContacts)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbb2d_data_offset);
    RETURN->v_int = c->GetNumContacts();
}

CK_DLL_MFUN(dbb2d_getContact)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbb2d_data_offset);
    int contactId = GET_NEXT_INT(ARGS);
    int bodyA, bodyB; 
    bool touching;
    int err = c->GetContact(contactId, &bodyA, &bodyB, &touching);
    if(!err)
    {
        RETURN->v_vec3.x = bodyA;
        RETURN->v_vec3.y = bodyB;
        RETURN->v_vec3.z = touching ? 1 : 0;
    }
    else
        RETURN->v_vec3.x = -1;
}

CK_DLL_MFUN( dbb2d_setGravity )
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbb2d_data_offset);
    t_CKCOMPLEX pos = GET_NEXT_COMPLEX(ARGS);
    c->SetGravity(pos);
}

CK_DLL_MFUN( dbb2d_setFriction )
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbb2d_data_offset);
    int bodyId = GET_NEXT_INT(ARGS);
    float x = GET_NEXT_FLOAT(ARGS);
    c->SetFriction(bodyId, x);
}

CK_DLL_MFUN( dbb2d_setDensity )
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbb2d_data_offset);
    int bodyId = GET_NEXT_INT(ARGS);
    float x = GET_NEXT_FLOAT(ARGS);
    c->SetDensity(bodyId, x);
}

CK_DLL_MFUN( dbb2d_setRestitution )
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbb2d_data_offset);
    int bodyId = GET_NEXT_INT(ARGS);
    float x = GET_NEXT_FLOAT(ARGS);
    c->SetRestitution(bodyId, x);
}

CK_DLL_MFUN( dbb2d_applyImpulse )
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbb2d_data_offset);
    int bodyId = GET_NEXT_INT(ARGS);
    t_CKCOMPLEX x = GET_NEXT_COMPLEX(ARGS);
    c->ApplyImpulse(bodyId, x);
}

CK_DLL_MFUN( dbb2d_applyAngularImpulse )
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbb2d_data_offset);
    int bodyId = GET_NEXT_INT(ARGS);
    float x = GET_NEXT_FLOAT(ARGS);
    c->ApplyAngularImpulse(bodyId, x);
}