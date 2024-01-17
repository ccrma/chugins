#include "WarpBufChugin.h"

// declaration of chugin constructor
CK_DLL_CTOR(warpbuf_ctor);
// declaration of chugin desctructor
CK_DLL_DTOR(warpbuf_dtor);

// functions
CK_DLL_MFUN(warpbuf_read);
CK_DLL_MFUN(warpbuf_reset);
CK_DLL_MFUN(warpbuf_getplayhead);
CK_DLL_MFUN(warpbuf_setplayhead);
CK_DLL_MFUN(warpbuf_getplay);
CK_DLL_MFUN(warpbuf_setplay);
CK_DLL_MFUN(warpbuf_getbpm);
CK_DLL_MFUN(warpbuf_setbpm);
CK_DLL_MFUN(warpbuf_gettranspose);
CK_DLL_MFUN(warpbuf_settranspose);
CK_DLL_MFUN(warpbuf_getstartmarker);
CK_DLL_MFUN(warpbuf_setstartmarker);
CK_DLL_MFUN(warpbuf_getendmarker);
CK_DLL_MFUN(warpbuf_setendmarker);
CK_DLL_MFUN(warpbuf_getloopenable);
CK_DLL_MFUN(warpbuf_setloopenable);
CK_DLL_MFUN(warpbuf_getloopstart);
CK_DLL_MFUN(warpbuf_setloopstart);
CK_DLL_MFUN(warpbuf_getloopend);
CK_DLL_MFUN(warpbuf_setloopend);

// multi-channel audio synthesis tick function
CK_DLL_TICKF(warpbuf_tick);

// this is a special offset reserved for Chugin internal data
t_CKINT warpbuf_data_offset = 0;

//-----------------------------------------------------------------------------
// query function: chuck calls this when loading the Chugin
//-----------------------------------------------------------------------------
CK_DLL_QUERY(WarpBuf)
{
    // hmm, don't change this...
    QUERY->setname(QUERY, "WarpBuf");

    // begin the class definition
    // can change the second argument to extend a different ChucK class
    QUERY->begin_class(QUERY, "WarpBuf", "UGen");

    // register the constructor (probably no need to change)
    QUERY->add_ctor(QUERY, warpbuf_ctor);
    // register the destructor (probably no need to change)
    QUERY->add_dtor(QUERY, warpbuf_dtor);

    QUERY->add_ugen_funcf(QUERY, warpbuf_tick, NULL, 0, WARPBUF_MAX_OUTPUTS);

    QUERY->add_mfun(QUERY, warpbuf_getplay, "int", "play");

    QUERY->add_mfun(QUERY, warpbuf_setplay, "int", "play");
    QUERY->add_arg(QUERY, "float", "play");

    QUERY->add_mfun(QUERY, warpbuf_read, "int", "read");
    QUERY->add_arg(QUERY, "string", "filename");

    QUERY->add_mfun(QUERY, warpbuf_reset, "int", "reset");

    QUERY->add_mfun(QUERY, warpbuf_getplayhead, "float", "playhead");
    QUERY->add_mfun(QUERY, warpbuf_setplayhead, "float", "playhead");
    QUERY->add_arg(QUERY, "float", "playhead");

    QUERY->add_mfun(QUERY, warpbuf_getbpm, "float", "bpm");
    QUERY->add_mfun(QUERY, warpbuf_setbpm, "float", "bpm");
    QUERY->add_arg(QUERY, "float", "bpm");

    QUERY->add_mfun(QUERY, warpbuf_gettranspose, "float", "transpose");
    QUERY->add_mfun(QUERY, warpbuf_settranspose, "float", "transpose");
    QUERY->add_arg(QUERY, "float", "transpose");

    QUERY->add_mfun(QUERY, warpbuf_getstartmarker, "float", "startMarker");
    QUERY->add_mfun(QUERY, warpbuf_setstartmarker, "float", "startMarker");
    QUERY->add_arg(QUERY, "float", "startMarker");

    QUERY->add_mfun(QUERY, warpbuf_getendmarker, "float", "endMarker");
    QUERY->add_mfun(QUERY, warpbuf_setendmarker, "float", "endMarker");
    QUERY->add_arg(QUERY, "float", "endMarker");

    QUERY->add_mfun(QUERY, warpbuf_getloopenable, "int", "loop");
    QUERY->add_mfun(QUERY, warpbuf_setloopenable, "int", "loop");
    QUERY->add_arg(QUERY, "int", "enable");

    QUERY->add_mfun(QUERY, warpbuf_getloopstart, "float", "loopStart");
    QUERY->add_mfun(QUERY, warpbuf_setloopstart, "float", "loopStart");
    QUERY->add_arg(QUERY, "float", "loopStart");

    QUERY->add_mfun(QUERY, warpbuf_getloopend, "float", "loopEnd");
    QUERY->add_mfun(QUERY, warpbuf_setloopend, "float", "loopEnd");
    QUERY->add_arg(QUERY, "float", "loopEnd");

    // this reserves a variable in the ChucK internal class to store
    // referene to the c++ class we defined above
    warpbuf_data_offset = QUERY->add_mvar(QUERY, "int", "@b_data", false);

    // end the class definition
    // IMPORTANT: this MUST be called!
    QUERY->end_class(QUERY);

    return TRUE;
}

// implementation for the constructor
CK_DLL_CTOR(warpbuf_ctor)
{
    // get the offset where we'll store our internal c++ class pointer
    OBJ_MEMBER_INT(SELF, warpbuf_data_offset) = 0;

    // instantiate our internal c++ class representation
    WarpBufChugin* b_obj = new WarpBufChugin(API->vm->srate(VM));

    // store the pointer in the ChucK object member
    OBJ_MEMBER_INT(SELF, warpbuf_data_offset) = (t_CKINT)b_obj;
}

// implementation for the destructor
CK_DLL_DTOR(warpbuf_dtor)
{
    // get our c++ class pointer
    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    // check it
    if (chug)
    {
        // clean up
        delete chug;
        OBJ_MEMBER_INT(SELF, warpbuf_data_offset) = 0;
        chug = NULL;
    }
}

// implementation for tick function
CK_DLL_TICKF(warpbuf_tick)
{
    // get our c++ class pointer
    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);

    // invoke our tick function; store in the magical out variable
    if (chug) chug->tick(in, out, nframes);

    // yes
    return TRUE;
}

CK_DLL_MFUN(warpbuf_reset)
{
    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    chug->reset();
    RETURN->v_int = 1;
}

CK_DLL_MFUN(warpbuf_getplayhead)
{
    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);

    RETURN->v_float = chug->getPlayhead();
}

CK_DLL_MFUN(warpbuf_setplayhead)
{
    float playhead = GET_NEXT_FLOAT(ARGS);

    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    chug->setPlayhead(playhead);
    RETURN->v_float = playhead;
}

CK_DLL_MFUN(warpbuf_getplay)
{
    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);

    RETURN->v_int = chug->getPlay();
}

CK_DLL_MFUN(warpbuf_setplay)
{
    bool play = GET_NEXT_FLOAT(ARGS);

    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);

    chug->setPlay(play);
    RETURN->v_int = play;
}

CK_DLL_MFUN(warpbuf_read)
{
    std::string filename = GET_NEXT_STRING_SAFE(ARGS);

    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);

    RETURN->v_int = chug->read(filename.c_str());
}

CK_DLL_MFUN(warpbuf_getbpm)
{
    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);

    RETURN->v_float = (float)chug->getBPM();
}

CK_DLL_MFUN(warpbuf_setbpm)
{
    t_CKFLOAT bpm = GET_NEXT_FLOAT(ARGS);

    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    chug->setBPM(bpm);
    RETURN->v_float = bpm;
}

CK_DLL_MFUN(warpbuf_gettranspose)
{
    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);

    RETURN->v_float = (float)chug->getTranspose();
}

CK_DLL_MFUN(warpbuf_settranspose)
{
    t_CKFLOAT transpose = GET_NEXT_FLOAT(ARGS);

    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    chug->setTranspose(transpose);
    RETURN->v_float = transpose;
}

CK_DLL_MFUN(warpbuf_getstartmarker)
{
    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);

    RETURN->v_float = chug->getStartMarker();
}

CK_DLL_MFUN(warpbuf_setstartmarker)
{
    t_CKFLOAT startMarker = GET_NEXT_FLOAT(ARGS);

    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    chug->setStartMarker(startMarker);
    RETURN->v_float = startMarker;
}

CK_DLL_MFUN(warpbuf_getendmarker)
{
    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);

    RETURN->v_float = chug->getEndMarker();
}

CK_DLL_MFUN(warpbuf_setendmarker)
{
    t_CKFLOAT endMarker = GET_NEXT_FLOAT(ARGS);

    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    chug->setEndMarker(endMarker);
    RETURN->v_float = endMarker;
}

CK_DLL_MFUN(warpbuf_getloopenable)
{
    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);

    RETURN->v_int = chug->getLoopEnable();
}

CK_DLL_MFUN(warpbuf_setloopenable)
{
    t_CKBOOL enable = GET_NEXT_INT(ARGS);

    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    chug->setLoopEnable(enable);
    RETURN->v_int = enable;
}

CK_DLL_MFUN(warpbuf_getloopstart)
{
    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);

    RETURN->v_float = chug->getLoopStart();
}

CK_DLL_MFUN(warpbuf_setloopstart)
{
    t_CKFLOAT loopStart = GET_NEXT_FLOAT(ARGS);

    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    chug->setLoopStart(loopStart);
    RETURN->v_float = loopStart;
}

CK_DLL_MFUN(warpbuf_getloopend)
{
    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);

    RETURN->v_float = chug->getLoopEnd();
}

CK_DLL_MFUN(warpbuf_setloopend)
{
    t_CKFLOAT loopEnd = GET_NEXT_FLOAT(ARGS);

    WarpBufChugin* chug = (WarpBufChugin*)OBJ_MEMBER_INT(SELF, warpbuf_data_offset);
    chug->setLoopEnd(loopEnd);
    RETURN->v_float = loopEnd;
}
