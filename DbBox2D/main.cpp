/*----------------------------------------------------------------------------
 See comments in DbBox2D.h

-----------------------------------------------------------------------------*/
#include "DbBox2D.h"

#include "chuck_dl.h"
#include "chuck_def.h"


CK_DLL_CTOR( dbld_ctor );
CK_DLL_DTOR( dbld_dtor );
CK_DLL_MFUN( dbld_worldBegin );
CK_DLL_MFUN( dbld_newEdge );
CK_DLL_MFUN( dbld_newCircle );
CK_DLL_MFUN( dbld_newTriangle );
CK_DLL_MFUN( dbld_newRectangle );
CK_DLL_MFUN( dbld_newRoom );
CK_DLL_MFUN( dbld_worldEnd );
CK_DLL_MFUN( dbld_done );
CK_DLL_MFUN( dbld_getPosition );
CK_DLL_MFUN( dbld_getVelocity );
CK_DLL_MFUN( dbld_getAngularVelocity );

CK_DLL_MFUN( dbld_setGravity );
CK_DLL_MFUN( dbld_setFriction );
CK_DLL_MFUN( dbld_setDensity );
CK_DLL_MFUN( dbld_setRestitution );
CK_DLL_MFUN( dbld_applyImpulse );
CK_DLL_MFUN( dbld_applyAngularImpulse );

t_CKINT dbld_data_offset = 0;

/* ----------------------------------------------------------- */
CK_DLL_QUERY(DbBox2D)
{
    QUERY->setname(QUERY, "DbBox2D");
    QUERY->begin_class(QUERY, "DbBox2D", "Event"); // we're not a ugen!

    QUERY->add_ctor(QUERY, dbld_ctor);
    QUERY->add_dtor(QUERY, dbld_dtor);

    QUERY->add_mfun(QUERY, dbld_worldBegin, "void", "worldBegin");

    QUERY->add_mfun(QUERY, dbld_worldEnd, "void", "worldEnd");

    QUERY->add_mfun(QUERY, dbld_newEdge, "int", "newEdge");
    QUERY->add_arg(QUERY, "complex", "p1");
    QUERY->add_arg(QUERY, "complex", "p2");
    QUERY->add_arg(QUERY, "int", "twoSided");

    QUERY->add_mfun(QUERY, dbld_newCircle, "int", "newCircle");
    QUERY->add_arg(QUERY, "complex", "position");
    QUERY->add_arg(QUERY, "float", "radius"); 
    QUERY->add_arg(QUERY, "int", "static");

    QUERY->add_mfun(QUERY, dbld_newTriangle, "int", "newTriangle");
    QUERY->add_arg(QUERY, "complex", "p1");
    QUERY->add_arg(QUERY, "complex", "p2");
    QUERY->add_arg(QUERY, "complex", "p3");
    QUERY->add_arg(QUERY, "int", "static");

    QUERY->add_mfun(QUERY, dbld_newRectangle, "int", "newRectangle");
    QUERY->add_arg(QUERY, "complex", "position");
    QUERY->add_arg(QUERY, "complex", "size");
    QUERY->add_arg(QUERY, "float", "angle");
    QUERY->add_arg(QUERY, "int", "static");

    QUERY->add_mfun(QUERY, dbld_newRoom, "int", "newRoom");
    QUERY->add_arg(QUERY, "complex", "position");
    QUERY->add_arg(QUERY, "complex", "size");
    QUERY->add_arg(QUERY, "float", "angle");
    QUERY->add_arg(QUERY, "int", "static");

    QUERY->add_mfun(QUERY, dbld_getPosition, "complex", "getPosition");
    QUERY->add_arg(QUERY, "int", "bodyId");

    QUERY->add_mfun(QUERY, dbld_getVelocity, "complex", "getVelocity");
    QUERY->add_arg(QUERY, "int", "bodyId");

    QUERY->add_mfun(QUERY, dbld_getAngularVelocity, "float", "getAngularVelocity");
    QUERY->add_arg(QUERY, "int", "bodyId");

    QUERY->add_mfun(QUERY, dbld_done, "void", "done");  // async

    // we'd like to invoke the broadcast method of our parent class

    dbld_data_offset = QUERY->add_mvar(QUERY, "int", "@dbld_data", false);
    QUERY->end_class(QUERY);
    return TRUE;
}

CK_DLL_CTOR(dbld_ctor)
{
    OBJ_MEMBER_INT(SELF, dbld_data_offset) = 0;
    DbBox2D *c = new DbBox2D((Chuck_Event *) SELF);

    // we are getting double-deleted, presumably because we are Event subclass
    // hack/fix is to add a ref so the first decref doesn't trigger
    SELF->add_ref(); // <---------------- hack, prevents dtor

    OBJ_MEMBER_INT(SELF, dbld_data_offset) = (t_CKINT) c;
}

CK_DLL_DTOR(dbld_dtor)
{
    std::cerr << "DbBox2D unexpected dtor\n";
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    if(c)
    {
        delete c;
        OBJ_MEMBER_INT(SELF, dbld_data_offset) = 0;
    }
}

CK_DLL_MFUN(dbld_worldBegin)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    t_CKCOMPLEX gravity = GET_NEXT_COMPLEX(ARGS);
    c->WorldBegin(gravity); 
}

CK_DLL_MFUN(dbld_worldEnd)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    c->WorldEnd();
}

CK_DLL_MFUN(dbld_newEdge)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    t_CKCOMPLEX p1 = GET_NEXT_COMPLEX(ARGS);
    t_CKCOMPLEX p2 = GET_NEXT_COMPLEX(ARGS);
    bool twoSided = GET_NEXT_INT(ARGS);
    RETURN->v_int = c->NewEdge(p1, p2, twoSided);
}

CK_DLL_MFUN(dbld_newCircle)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    t_CKCOMPLEX pos = GET_NEXT_COMPLEX(ARGS);
    float radius = GET_NEXT_FLOAT(ARGS);
    float density = GET_NEXT_FLOAT(ARGS);
    int bt = GET_NEXT_INT(ARGS);
    DbBox2D::BodyType t = DbBox2D::k_Static;
    if(bt >= 0 && bt < DbBox2D::k_NumBodyTypes)
        t = (DbBox2D::BodyType) bt;
    RETURN->v_int = c->NewCircle(pos, radius, density, t);
}

CK_DLL_MFUN(dbld_newTriangle)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
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

CK_DLL_MFUN(dbld_newRectangle)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    t_CKCOMPLEX pos = GET_NEXT_COMPLEX(ARGS);
    t_CKCOMPLEX sz = GET_NEXT_COMPLEX(ARGS);
    float density = GET_NEXT_FLOAT(ARGS);
    int bt = GET_NEXT_INT(ARGS);
    DbBox2D::BodyType t = DbBox2D::k_Static;
    if(bt >= 0 && bt < DbBox2D::k_NumBodyTypes)
        t = (DbBox2D::BodyType) bt;
    RETURN->v_int = c->NewRectangle(pos, sz, density, t);
}

CK_DLL_MFUN(dbld_newRoom)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    t_CKCOMPLEX pos = GET_NEXT_COMPLEX(ARGS);
    t_CKCOMPLEX sz = GET_NEXT_COMPLEX(ARGS);
    float density = GET_NEXT_FLOAT(ARGS);
    int bt = GET_NEXT_INT(ARGS);
    DbBox2D::BodyType t = DbBox2D::k_Static;
    if(bt >= 0 && bt < DbBox2D::k_NumBodyTypes)
        t = (DbBox2D::BodyType) bt;
    RETURN->v_int = c->NewRoom(pos, sz, density, t);
}

CK_DLL_MFUN(dbld_done)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    c->Done();
}

CK_DLL_MFUN(dbld_getPosition)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    int bodyId = GET_NEXT_INT(ARGS);
    c->GetPosition(bodyId, RETURN->v_complex);
}

CK_DLL_MFUN(dbld_getVelocity)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    int bodyId = GET_NEXT_INT(ARGS);
    c->GetVelocity(bodyId, RETURN->v_complex);
}

CK_DLL_MFUN(dbld_getAngularVelocity)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    int bodyId = GET_NEXT_INT(ARGS);
    RETURN->v_float = c->GetAngularVelocity(bodyId);
}

CK_DLL_MFUN( dbld_setGravity )
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    t_CKCOMPLEX pos = GET_NEXT_COMPLEX(ARGS);
    c->SetGravity(pos);
}

CK_DLL_MFUN( dbld_setFriction )
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    int bodyId = GET_NEXT_INT(ARGS);
    float x = GET_NEXT_FLOAT(ARGS);
    c->SetFriction(bodyId, x);
}

CK_DLL_MFUN( dbld_setDensity )
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    int bodyId = GET_NEXT_INT(ARGS);
    float x = GET_NEXT_FLOAT(ARGS);
    c->SetDensity(bodyId, x);
}

CK_DLL_MFUN( dbld_setRestitution )
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    int bodyId = GET_NEXT_INT(ARGS);
    float x = GET_NEXT_FLOAT(ARGS);
    c->SetRestitution(bodyId, x);
}

CK_DLL_MFUN( dbld_applyImpulse )
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    int bodyId = GET_NEXT_INT(ARGS);
    t_CKCOMPLEX x = GET_NEXT_COMPLEX(ARGS);
    c->ApplyImpulse(bodyId, x);
}

CK_DLL_MFUN( dbld_applyAngularImpulse )
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    int bodyId = GET_NEXT_INT(ARGS);
    float x = GET_NEXT_FLOAT(ARGS);
    c->ApplyAngularImpulse(bodyId, x);
}