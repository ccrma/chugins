/*----------------------------------------------------------------------------
 See comments in DbImgSampler.h

-----------------------------------------------------------------------------*/
#include "DbImgSampler.h"

#include "chuck_dl.h"
#include "chuck_def.h"


CK_DLL_CTOR( dbld_ctor );
CK_DLL_DTOR( dbld_dtor );
CK_DLL_MFUN( dbld_loadImage );
CK_DLL_MFUN( dbld_getSample );
t_CKINT dbld_data_offset = 0;

/* ----------------------------------------------------------- */
CK_DLL_QUERY(DbImgSampelr)
{
    QUERY->setname(QUERY, "DbImgSampler");
    QUERY->begin_class(QUERY, "DbImgSampler", "Object"); // we're not a ugen!

    QUERY->add_ctor(QUERY, dbld_ctor);
    QUERY->add_dtor(QUERY, dbld_dtor);

    QUERY->add_mfun(QUERY, dbld_loadImage, "void", "loadImage");  // async
    QUERY->add_arg(QUERY, "string", "imageFile");

    QUERY->add_mfun(QUERY, dbld_getSample, "int", "getSample");
    QUERY->add_arg(QUERY, "float", "x"); // pct [0-1]
    QUERY->add_arg(QUERY, "float", "y"); // pct [0-1]

    dbld_data_offset = QUERY->add_mvar(QUERY, "int", "@dbld_data", false);
    QUERY->end_class(QUERY);
    return TRUE;
}

CK_DLL_CTOR(dbld_ctor)
{
    OBJ_MEMBER_INT(SELF, dbld_data_offset) = 0;
    DbImgSampler *c = new DbImgSampler();
    OBJ_MEMBER_INT(SELF, dbld_data_offset) = (t_CKINT) c;
}

CK_DLL_DTOR(dbld_dtor)
{
    DbImgSampler *c = (DbImgSampler *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    if(c)
    {
        delete c;
        OBJ_MEMBER_INT(SELF, dbld_data_offset) = 0;
        c = NULL;
    }
}

CK_DLL_MFUN(dbld_loadImage)
{
    DbImgSampler *c = (DbImgSampler *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    std::string filename = GET_NEXT_STRING_SAFE(ARGS);
    char const *cp = filename.c_str();
    c->Load(cp); // async, no return;
}

CK_DLL_MFUN(dbld_getSample)
{
    DbImgSampler *c = (DbImgSampler *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    std::string filename = GET_NEXT_STRING_SAFE(ARGS);
    int result;
    float x = GET_NEXT_FLOAT(ARGS);
    float y = GET_NEXT_FLOAT(ARGS);
    int err = c->GetSample(x, y, &result);
    RETURN->v_int = result;
}