/*----------------------------------------------------------------------------
 See comments in DbSpectral.h

-----------------------------------------------------------------------------*/
#include "DbSpectral.h"

#include "chuck_dl.h"
#include "chuck_def.h"


/* our parameters ---
 *   loadSpectrogram
 *   loadTiming (?)
 */

CK_DLL_CTOR( dbld_ctor );
CK_DLL_DTOR( dbld_dtor );
CK_DLL_MFUN( dbld_init );
CK_DLL_MFUN( dbld_loadSpectrogram );
CK_DLL_TICK( dbld_tick );

t_CKINT dbld_data_offset = 0;

/* ----------------------------------------------------------- */
CK_DLL_QUERY(DbSpectral)
{
    QUERY->setname(QUERY, "DbSpectral");
    QUERY->begin_class(QUERY, "DbSpectral", "UGen"); // following Pan4 example

    QUERY->add_ctor(QUERY, dbld_ctor);
    QUERY->add_dtor(QUERY, dbld_dtor);
    QUERY->add_ugen_func(QUERY, dbld_tick, NULL, 1, 1);

    QUERY->add_mfun(QUERY, dbld_init, "void", "init");
    QUERY->add_arg(QUERY, "int", "fftSize");
    QUERY->add_arg(QUERY, "int", "overlapSize");

    dbld_data_offset = QUERY->add_mvar(QUERY, "int", "@dbld_data", false);
    QUERY->end_class(QUERY);
    return TRUE;
}

CK_DLL_CTOR(dbld_ctor)
{
    OBJ_MEMBER_INT(SELF, dbld_data_offset) = 0;
    DbSpectral *c = new DbSpectral(API->vm->get_srate(API, SHRED));
    OBJ_MEMBER_INT(SELF, dbld_data_offset) = (t_CKINT) c;
    c->Init(512, 128);
}

CK_DLL_DTOR(dbld_dtor)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    if(c)
    {
        delete c;
        OBJ_MEMBER_INT(SELF, dbld_data_offset) = 0;
        c = NULL;
    }
}

CK_DLL_MFUN(dbld_init)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    int fftSize = GET_NEXT_INT(ARGS); //
    int overlap = GET_NEXT_INT(ARGS); //
    c->Init(fftSize, overlap);
}

CK_DLL_TICK(dbld_tick)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    *out = c->Tick(in);
    return TRUE;
}

CK_DLL_MFUN(dbld_loadSpectrogram)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    std::string filename = GET_NEXT_STRING_SAFE(ARGS);
    c->LoadSpectogram(filename.c_str()); // async, no return code
}
