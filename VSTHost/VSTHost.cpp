

#include "chuck_dl.h"
#include "chuck_def.h"

#include <stdio.h>
#include <limits.h>


CK_DLL_CTOR(vsthost_ctor);
CK_DLL_DTOR(vsthost_dtor);

CK_DLL_MFUN(vsthost_setParam);
CK_DLL_MFUN(vsthost_getParam);

CK_DLL_TICK(vsthost_tick);

t_CKINT vsthost_data_offset = 0;


class VSTHost
{
public:
    
    VSTHost(float fs)
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

CK_DLL_QUERY(VSTHost)
{
    QUERY->setname(QUERY, "VSTHost");
    
    QUERY->begin_class(QUERY, "VSTHost", "UGen");
    
    QUERY->add_ctor(QUERY, vsthost_ctor);
    QUERY->add_dtor(QUERY, vsthost_dtor);
    
    QUERY->add_ugen_func(QUERY, vsthost_tick, NULL, 1, 1);
    
    QUERY->add_mfun(QUERY, vsthost_setParam, "float", "param");
    QUERY->add_arg(QUERY, "float", "arg");
    
    QUERY->add_mfun(QUERY, vsthost_getParam, "float", "param");
    
    vsthost_data_offset = QUERY->add_mvar(QUERY, "int", "@vsthost_data", false);
    
    QUERY->end_class(QUERY);

    return TRUE;
}


CK_DLL_CTOR(vsthost_ctor)
{
    OBJ_MEMBER_INT(SELF, vsthost_data_offset) = 0;
    
    VSTHost * bcdata = new VSTHost(API->vm->get_srate());
    
    OBJ_MEMBER_INT(SELF, vsthost_data_offset) = (t_CKINT) bcdata;
}

CK_DLL_DTOR(vsthost_dtor)
{
    VSTHost * bcdata = (VSTHost *) OBJ_MEMBER_INT(SELF, vsthost_data_offset);
    if(bcdata)
    {
        delete bcdata;
        OBJ_MEMBER_INT(SELF, vsthost_data_offset) = 0;
        bcdata = NULL;
    }
}

CK_DLL_TICK(vsthost_tick)
{
    VSTHost * c = (VSTHost *) OBJ_MEMBER_INT(SELF, vsthost_data_offset);
    
    if(c) *out = c->tick(in);

    return TRUE;
}

CK_DLL_MFUN(vsthost_setParam)
{
    VSTHost * bcdata = (VSTHost *) OBJ_MEMBER_INT(SELF, vsthost_data_offset);
    // TODO: sanity check
    RETURN->v_float = bcdata->setParam(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(vsthost_getParam)
{
    VSTHost * bcdata = (VSTHost *) OBJ_MEMBER_INT(SELF, vsthost_data_offset);
    RETURN->v_float = bcdata->getParam();
}

