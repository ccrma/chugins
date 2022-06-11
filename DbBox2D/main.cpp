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
CK_DLL_MFUN( dbld_getNumEvents );
CK_DLL_MFUN( dbld_getEvent );


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
    std::string filename = GET_NEXT_STRING_SAFE(ARGS);
    char const *cp = filename.c_str();
    RETURN->v_int = c->LoadWorld(cp); // async, nonzero return indicates busy
}

CK_DLL_MFUN(dbld_done)
{
    DbBox2D *c = (DbBox2D *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    c->Done();
}