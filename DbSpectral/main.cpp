/*----------------------------------------------------------------------------
 See comments in DbSpectral.h

-----------------------------------------------------------------------------*/
#include "DbSpectral.h"

#include "chuck_dl.h"
#include "chuck_def.h"


CK_DLL_CTOR( dbld_ctor );
CK_DLL_DTOR( dbld_dtor );
CK_DLL_MFUN( dbld_init );
CK_DLL_TICK( dbld_tick );
CK_DLL_MFUN( dbld_loadSpectogram );
CK_DLL_MFUN( dbld_getColumn );
CK_DLL_MFUN( dbld_getColumnPct );
CK_DLL_MFUN( dbld_scanRate );
CK_DLL_MFUN( dbld_freqMin );
CK_DLL_MFUN( dbld_freqMax );
CK_DLL_MFUN( dbld_delayMax );
CK_DLL_MFUN( dbld_feedbackMin );
CK_DLL_MFUN( dbld_feedbackMax );
CK_DLL_MFUN( dbld_verbosity );

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
    QUERY->add_arg(QUERY, "int", "mode");

    QUERY->add_mfun(QUERY, dbld_freqMin, "void", "freqMin");
    QUERY->add_arg(QUERY, "int", "minFreq");

    QUERY->add_mfun(QUERY, dbld_freqMax, "void", "freqMax");
    QUERY->add_arg(QUERY, "int", "maxFreq");

    QUERY->add_mfun(QUERY, dbld_delayMax, "void", "delayMax");
    QUERY->add_arg(QUERY, "float", "delayMax");

    QUERY->add_mfun(QUERY, dbld_feedbackMin, "void", "feedbackMin");
    QUERY->add_arg(QUERY, "float", "feedbackMin");

    QUERY->add_mfun(QUERY, dbld_feedbackMax, "void", "feedbackMax");
    QUERY->add_arg(QUERY, "float", "feedbackMax");

    QUERY->add_mfun(QUERY, dbld_loadSpectogram, "void", "loadSpectogram"); // legacy alias
    QUERY->add_arg(QUERY, "string", "imageFile");

    QUERY->add_mfun(QUERY, dbld_scanRate, "void", "scanRate");
    QUERY->add_arg(QUERY, "int", "rate"); // columns per second

    QUERY->add_mfun(QUERY, dbld_getColumn, "int", "getColumn");
    // no parameters

    QUERY->add_mfun(QUERY, dbld_getColumnPct, "float", "getColumnPct");
    // no parameters

    QUERY->add_mfun(QUERY, dbld_verbosity, "void", "verbosity");
    QUERY->add_arg(QUERY, "int", "verbosity");

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
    int mode = GET_NEXT_INT(ARGS); //
    c->Init(fftSize, overlap, (DbSpectral::ImgModes) mode);
}

CK_DLL_TICK(dbld_tick)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    *out = c->Tick(in);
    return TRUE;
}

CK_DLL_MFUN(dbld_loadSpectogram)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    std::string filename = GET_NEXT_STRING_SAFE(ARGS);
    c->LoadSpectralImage(filename.c_str()); // asynchronous, so no return
}

CK_DLL_MFUN(dbld_getColumn)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    RETURN->v_int = c->GetColumn();
}

CK_DLL_MFUN(dbld_getColumnPct)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    RETURN->v_float = c->GetColumnPct();
}

CK_DLL_MFUN(dbld_scanRate)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    int rate = GET_NEXT_INT(ARGS);
    c->SetScanRate(rate);
}

CK_DLL_MFUN(dbld_freqMin)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    int min = GET_NEXT_INT(ARGS);
    c->SetFreqMin(min);
}

CK_DLL_MFUN(dbld_freqMax)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    int max = GET_NEXT_INT(ARGS);
    c->SetFreqMax(max);
}

CK_DLL_MFUN(dbld_delayMax)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    float max = GET_NEXT_FLOAT(ARGS);
    c->SetDelayMax(max);
}

CK_DLL_MFUN(dbld_feedbackMin)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    float min = GET_NEXT_FLOAT(ARGS);
    c->SetFeedbackMin(min);
}

CK_DLL_MFUN(dbld_feedbackMax)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    float max = GET_NEXT_FLOAT(ARGS);
    c->SetFeedbackMax(max);
}

CK_DLL_MFUN(dbld_verbosity)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbld_data_offset);
    int x = GET_NEXT_INT(ARGS);
    c->SetVerbosity(x);
}