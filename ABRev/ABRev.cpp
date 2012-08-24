

#include "chuck_dl.h"
#include "chuck_def.h"

#include <stdio.h>
#include <limits.h>


CK_DLL_CTOR(abrev_ctor);
CK_DLL_DTOR(abrev_dtor);

CK_DLL_MFUN(abrev_setParam);
CK_DLL_MFUN(abrev_getParam);

CK_DLL_TICK(abrev_tick);

t_CKINT abrev_data_offset = 0;


class ABRev
{
public:
    
    ABRev(float fs)
    {
        m_param = 0;
    }
    
    SAMPLE tick(SAMPLE in)
    {
        return in;
    }
    
    float setParam(float p)
    {
        m_param = p;
        return p;
    }
    
    float getParam() { return m_param; }
    
private:
    
    float m_param;
};

CK_DLL_QUERY(ABRev)
{
    QUERY->setname(QUERY, "ABRev");
    
    QUERY->begin_class(QUERY, "ABRev", "UGen");
    
    QUERY->add_ctor(QUERY, abrev_ctor);
    QUERY->add_dtor(QUERY, abrev_dtor);
    
    QUERY->add_ugen_func(QUERY, abrev_tick, NULL, 1, 1);
    
    QUERY->add_mfun(QUERY, abrev_setParam, "float", "param");
    QUERY->add_arg(QUERY, "float", "arg");
    
    QUERY->add_mfun(QUERY, abrev_getParam, "float", "param");
    
    abrev_data_offset = QUERY->add_mvar(QUERY, "int", "@abrev_data", false);
    
    QUERY->end_class(QUERY);

    return TRUE;
}


CK_DLL_CTOR(abrev_ctor)
{
    OBJ_MEMBER_INT(SELF, abrev_data_offset) = 0;
    
    ABRev * bcdata = new ABRev(API->vm->get_srate());
    
    OBJ_MEMBER_INT(SELF, abrev_data_offset) = (t_CKINT) bcdata;
}

CK_DLL_DTOR(abrev_dtor)
{
    ABRev * bcdata = (ABRev *) OBJ_MEMBER_INT(SELF, abrev_data_offset);
    if(bcdata)
    {
        delete bcdata;
        OBJ_MEMBER_INT(SELF, abrev_data_offset) = 0;
        bcdata = NULL;
    }
}

CK_DLL_TICK(abrev_tick)
{
    ABRev * c = (ABRev *) OBJ_MEMBER_INT(SELF, abrev_data_offset);
    
    if(c) *out = c->tick(in);

    return TRUE;
}

CK_DLL_MFUN(abrev_setParam)
{
    ABRev * bcdata = (ABRev *) OBJ_MEMBER_INT(SELF, abrev_data_offset);
    // TODO: sanity check
    RETURN->v_float = bcdata->setParam(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(abrev_getParam)
{
    ABRev * bcdata = (ABRev *) OBJ_MEMBER_INT(SELF, abrev_data_offset);
    RETURN->v_float = bcdata->getParam();
}

