

#include "chuck_dl.h"
#include "chuck_def.h"

#include <stdio.h>
#include <limits.h>


CK_DLL_CTOR(%(CHUGIN_LCNAME)%_ctor);
CK_DLL_DTOR(%(CHUGIN_LCNAME)%_dtor);

CK_DLL_MFUN(%(CHUGIN_LCNAME)%_setParam);
CK_DLL_MFUN(%(CHUGIN_LCNAME)%_getParam);

CK_DLL_TICK(%(CHUGIN_LCNAME)%_tick);

t_CKINT %(CHUGIN_LCNAME)%_data_offset = 0;


class %(CHUGIN_NAME)%
{
public:
    
    %(CHUGIN_NAME)%(float fs)
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

CK_DLL_QUERY(%(CHUGIN_NAME)%)
{
    QUERY->setname(QUERY, "%(CHUGIN_NAME)%");
    
    QUERY->begin_class(QUERY, "%(CHUGIN_NAME)%", "UGen");
    
    QUERY->add_ctor(QUERY, %(CHUGIN_LCNAME)%_ctor);
    QUERY->add_dtor(QUERY, %(CHUGIN_LCNAME)%_dtor);
    
    QUERY->add_ugen_func(QUERY, %(CHUGIN_LCNAME)%_tick, NULL, 1, 1);
    
    QUERY->add_mfun(QUERY, %(CHUGIN_LCNAME)%_setParam, "float", "param");
    QUERY->add_arg(QUERY, "float", "arg");
    
    QUERY->add_mfun(QUERY, %(CHUGIN_LCNAME)%_getParam, "float", "param");
    
    %(CHUGIN_LCNAME)%_data_offset = QUERY->add_mvar(QUERY, "int", "@%(CHUGIN_INITIALS)%_data", false);
    
    QUERY->end_class(QUERY);

    return TRUE;
}


CK_DLL_CTOR(%(CHUGIN_LCNAME)%_ctor)
{
    OBJ_MEMBER_INT(SELF, %(CHUGIN_LCNAME)%_data_offset) = 0;
    
    %(CHUGIN_NAME)% * bcdata = new %(CHUGIN_NAME)%(API->vm->get_srate());
    
    OBJ_MEMBER_INT(SELF, %(CHUGIN_LCNAME)%_data_offset) = (t_CKINT) bcdata;
}

CK_DLL_DTOR(%(CHUGIN_LCNAME)%_dtor)
{
    %(CHUGIN_NAME)% * bcdata = (%(CHUGIN_NAME)% *) OBJ_MEMBER_INT(SELF, %(CHUGIN_LCNAME)%_data_offset);
    if(bcdata)
    {
        delete bcdata;
        OBJ_MEMBER_INT(SELF, %(CHUGIN_LCNAME)%_data_offset) = 0;
        bcdata = NULL;
    }
}

CK_DLL_TICK(%(CHUGIN_LCNAME)%_tick)
{
    %(CHUGIN_NAME)% * c = (%(CHUGIN_NAME)% *) OBJ_MEMBER_INT(SELF, %(CHUGIN_LCNAME)%_data_offset);
    
    if(c) *out = c->tick(in);

    return TRUE;
}

CK_DLL_MFUN(%(CHUGIN_LCNAME)%_setParam)
{
    %(CHUGIN_NAME)% * bcdata = (%(CHUGIN_NAME)% *) OBJ_MEMBER_INT(SELF, %(CHUGIN_LCNAME)%_data_offset);
    // TODO: sanity check
    RETURN->v_float = bcdata->setParam(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(%(CHUGIN_LCNAME)%_getParam)
{
    %(CHUGIN_NAME)% * bcdata = (%(CHUGIN_NAME)% *) OBJ_MEMBER_INT(SELF, %(CHUGIN_LCNAME)%_data_offset);
    RETURN->v_float = bcdata->getParam();
}

