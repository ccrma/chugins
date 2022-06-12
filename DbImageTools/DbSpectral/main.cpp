/*----------------------------------------------------------------------------
 See comments in DbSpectral.h

-----------------------------------------------------------------------------*/
#include "DbSpectral.h"

#include "chuck_dl.h"
#include "chuck_def.h"


CK_DLL_CTOR( dbsp_ctor );
CK_DLL_DTOR( dbsp_dtor );
CK_DLL_MFUN( dbsp_init );
CK_DLL_TICK( dbsp_tick );
CK_DLL_MFUN( dbsp_mix );
CK_DLL_MFUN( dbsp_loadSpectogram );
CK_DLL_MFUN( dbsp_getColumn );
CK_DLL_MFUN( dbsp_getColumnPct );
CK_DLL_MFUN( dbsp_scanRate );
CK_DLL_MFUN( dbsp_freqMin );
CK_DLL_MFUN( dbsp_freqMax );
CK_DLL_MFUN( dbsp_delayMax );
CK_DLL_MFUN( dbsp_feedbackMax );
CK_DLL_MFUN( dbsp_verbosity );

t_CKINT dbsp_data_offset = 0;

/* ----------------------------------------------------------- */
CK_DLL_QUERY(DbSpectral)
{
    QUERY->setname(QUERY, "DbSpectral");
    QUERY->begin_class(QUERY, "DbSpectral", "UGen"); // following Pan4 example

    QUERY->add_ctor(QUERY, dbsp_ctor);
    QUERY->add_dtor(QUERY, dbsp_dtor);
    QUERY->add_ugen_func(QUERY, dbsp_tick, NULL, 1, 1);

    QUERY->add_mfun(QUERY, dbsp_init, "void", "init");
    QUERY->add_arg(QUERY, "int", "fftSize");
    QUERY->add_arg(QUERY, "int", "overlapSize");
    QUERY->add_arg(QUERY, "int", "mode");

    QUERY->add_mfun(QUERY, dbsp_mix, "void", "mix");
    QUERY->add_arg(QUERY, "float", "x");

    QUERY->add_mfun(QUERY, dbsp_freqMin, "void", "freqMin");
    QUERY->add_arg(QUERY, "int", "minFreq");

    QUERY->add_mfun(QUERY, dbsp_freqMax, "void", "freqMax");
    QUERY->add_arg(QUERY, "int", "maxFreq");

    QUERY->add_mfun(QUERY, dbsp_delayMax, "void", "delayMax");
    QUERY->add_arg(QUERY, "float", "delayMax");

    QUERY->add_mfun(QUERY, dbsp_feedbackMax, "void", "feedbackMax");
    QUERY->add_arg(QUERY, "float", "feedbackMax");

    QUERY->add_mfun(QUERY, dbsp_loadSpectogram, "void", "loadSpectogram"); // legacy alias
    QUERY->add_arg(QUERY, "string", "imageFile");

    QUERY->add_mfun(QUERY, dbsp_scanRate, "void", "scanRate");
    QUERY->add_arg(QUERY, "int", "rate"); // columns per second

    QUERY->add_mfun(QUERY, dbsp_getColumn, "int", "getColumn");
    // no parameters

    QUERY->add_mfun(QUERY, dbsp_getColumnPct, "float", "getColumnPct");
    // no parameters

    QUERY->add_mfun(QUERY, dbsp_verbosity, "void", "verbosity");
    QUERY->add_arg(QUERY, "int", "verbosity");

    dbsp_data_offset = QUERY->add_mvar(QUERY, "int", "@dbsp_data", false);
    QUERY->end_class(QUERY);
    return TRUE;
}

CK_DLL_CTOR(dbsp_ctor)
{
    OBJ_MEMBER_INT(SELF, dbsp_data_offset) = 0;
    DbSpectral *c = new DbSpectral(API->vm->get_srate(API, SHRED));
    OBJ_MEMBER_INT(SELF, dbsp_data_offset) = (t_CKINT) c;
    c->Init(512, 128);
}

CK_DLL_DTOR(dbsp_dtor)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbsp_data_offset);
    if(c)
    {
        delete c;
        OBJ_MEMBER_INT(SELF, dbsp_data_offset) = 0;
        c = NULL;
    }
}

CK_DLL_MFUN(dbsp_init)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbsp_data_offset);
    int fftSize = GET_NEXT_INT(ARGS); //
    int overlap = GET_NEXT_INT(ARGS); //
    int mode = GET_NEXT_INT(ARGS); //
    c->Init(fftSize, overlap, (DbSpectral::ImgModes) mode);
}

CK_DLL_TICK(dbsp_tick)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbsp_data_offset);
    *out = c->Tick(in);
    return TRUE;
}

CK_DLL_MFUN(dbsp_mix)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbsp_data_offset);
    c->SetMix(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(dbsp_loadSpectogram)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbsp_data_offset);
    std::string filename = GET_NEXT_STRING_SAFE(ARGS);
    c->LoadSpectralImage(filename.c_str()); // asynchronous, so no return
}

CK_DLL_MFUN(dbsp_getColumn)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbsp_data_offset);
    RETURN->v_int = c->GetColumn();
}

CK_DLL_MFUN(dbsp_getColumnPct)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbsp_data_offset);
    RETURN->v_float = c->GetColumnPct();
}

CK_DLL_MFUN(dbsp_scanRate)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbsp_data_offset);
    int rate = GET_NEXT_INT(ARGS);
    c->SetScanRate(rate);
}

CK_DLL_MFUN(dbsp_freqMin)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbsp_data_offset);
    int min = GET_NEXT_INT(ARGS);
    c->SetFreqMin(min);
}

CK_DLL_MFUN(dbsp_freqMax)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbsp_data_offset);
    int max = GET_NEXT_INT(ARGS);
    c->SetFreqMax(max);
}

CK_DLL_MFUN(dbsp_delayMax)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbsp_data_offset);
    float max = GET_NEXT_FLOAT(ARGS);
    c->SetDelayMax(max);
}

CK_DLL_MFUN(dbsp_feedbackMax)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbsp_data_offset);
    float max = GET_NEXT_FLOAT(ARGS);
    c->SetFeedbackMax(max);
}

CK_DLL_MFUN(dbsp_verbosity)
{
    DbSpectral *c = (DbSpectral *) OBJ_MEMBER_INT(SELF, dbsp_data_offset);
    int x = GET_NEXT_INT(ARGS);
    c->SetVerbosity(x);
}