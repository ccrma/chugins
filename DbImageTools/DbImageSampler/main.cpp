/*----------------------------------------------------------------------------
 See comments in DbImageSampler.h

-----------------------------------------------------------------------------*/
#include "DbImageSampler.h"

#include "chuck_dl.h"
#include "chuck_def.h"


CK_DLL_CTOR( dbis_ctor );
CK_DLL_DTOR( dbis_dtor );
CK_DLL_MFUN( dbis_loadImage );
CK_DLL_MFUN( dbis_getSample );
t_CKINT dbis_data_offset = 0;

/* ----------------------------------------------------------- */
CK_DLL_QUERY(DbImageSampelr)
{
    QUERY->setname(QUERY, "DbImageSampler");
    QUERY->begin_class(QUERY, "DbImageSampler", "Object"); // we're not a ugen!

    QUERY->add_ctor(QUERY, dbis_ctor);
    QUERY->add_dtor(QUERY, dbis_dtor);

    QUERY->add_mfun(QUERY, dbis_loadImage, "void", "loadImage");  // async
    QUERY->add_arg(QUERY, "string", "imageFile");

    QUERY->add_mfun(QUERY, dbis_getSample, "vec4", "getSample");
    QUERY->add_arg(QUERY, "float", "x"); // pct [0-1]
    QUERY->add_arg(QUERY, "float", "y"); // pct [0-1]

    dbis_data_offset = QUERY->add_mvar(QUERY, "int", "@dbis_data", false);
    QUERY->end_class(QUERY);
    return TRUE;
}

CK_DLL_CTOR(dbis_ctor)
{
    OBJ_MEMBER_INT(SELF, dbis_data_offset) = 0;
    DbImageSampler *c = new DbImageSampler();
    OBJ_MEMBER_INT(SELF, dbis_data_offset) = (t_CKINT) c;
}

CK_DLL_DTOR(dbis_dtor)
{
    DbImageSampler *c = (DbImageSampler *) OBJ_MEMBER_INT(SELF, dbis_data_offset);
    if(c)
    {
        delete c;
        OBJ_MEMBER_INT(SELF, dbis_data_offset) = 0;
        c = NULL;
    }
}

CK_DLL_MFUN(dbis_loadImage)
{
    DbImageSampler *c = (DbImageSampler *) OBJ_MEMBER_INT(SELF, dbis_data_offset);
    std::string filename = GET_NEXT_STRING_SAFE(ARGS);
    char const *cp = filename.c_str();
    c->Load(cp); // async, no return;
}

CK_DLL_MFUN(dbis_getSample)
{
    DbImageSampler *c = (DbImageSampler *) OBJ_MEMBER_INT(SELF, dbis_data_offset);
    float x = GET_NEXT_FLOAT(ARGS);
    float y = GET_NEXT_FLOAT(ARGS);
    float r, g, b, a;
    c->GetSample(x, y, &r, &g, &b, &a);
    RETURN->v_vec4.x = r;
    RETURN->v_vec4.y = g;
    RETURN->v_vec4.z = b;
    RETURN->v_vec4.w = a;
}