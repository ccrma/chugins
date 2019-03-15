// this should align with the correct versions of these ChucK files
#include "chuck_dl.h"
#include "chuck_def.h"

// general includes
#include <stdio.h>
#include <limits.h>
#include <math.h>

// declaration of chugin constructor
CK_DLL_CTOR(duffing_ctor);
// declaration of chugin destructor
CK_DLL_DTOR(duffing_dtor);

// getters/setters
CK_DLL_MFUN(duffing_setAlpha);
CK_DLL_MFUN(duffing_getAlpha);
CK_DLL_MFUN(duffing_setBeta);
CK_DLL_MFUN(duffing_getBeta);
CK_DLL_MFUN(duffing_setDelta);
CK_DLL_MFUN(duffing_getDelta);
CK_DLL_MFUN(duffing_setGamma);
CK_DLL_MFUN(duffing_getGamma);
CK_DLL_MFUN(duffing_setOmega);
CK_DLL_MFUN(duffing_getOmega);
CK_DLL_MFUN(duffing_setStep);
CK_DLL_MFUN(duffing_getStep);
CK_DLL_MFUN(duffing_setX);
CK_DLL_MFUN(duffing_getX);
CK_DLL_MFUN(duffing_setV);
CK_DLL_MFUN(duffing_getV);

// reset
CK_DLL_MFUN(duffing_reset);

// for Chugins extending UGen, this is mono synthesis function for 1 sample
CK_DLL_TICK(duffing_tick);

// this is a special offset reserved for Chugin internal data
t_CKINT duffing_data_offset = 0;

// class definition for internal Chugin data
class Duffing
{
public:
    // constructor
    Duffing( t_CKFLOAT fs)
    {
        m_alpha = 1.0;   // stiffness
        m_beta  = 5.0;   // non-linearity in restoring force
        m_delta = 0.02;  // damping 
        m_gamma = 8.0;   // amplitude of driving force
        m_omega = 0.5;   // frequency of driving force

        // initial conditions
        m_t = 0.0;
        m_x = 0.0;
        m_v = 0.0;

        m_h = 1.0 / fs;
    }

    // for Chugins extending UGen
    SAMPLE tick( SAMPLE in )
    {
        // This is the step function for RK4, the classical Runge-Kutta method,
        // but omitting the unnecessary function call to f1(t, x, v) for the kx*
        // coefficients, as it always returns v.
        t_CKFLOAT kx0 = m_v;
        t_CKFLOAT kv0 = f2(m_t, m_x, m_v);
        t_CKFLOAT kx1 = m_h * (m_v + kv0 / 2);
        t_CKFLOAT kv1 = m_h * f2(m_t + m_h / 2, m_x + kx0 / 2, m_v + kv0 / 2);
        t_CKFLOAT kx2 = m_h * (m_v + kv1 / 2);
        t_CKFLOAT kv2 = m_h * f2(m_t + m_h / 2, m_x + kx1 / 2, m_v + kv1 / 2);
        t_CKFLOAT kx3 = m_h * (m_v + kv2);
        t_CKFLOAT kv3 = m_h * f2(m_t + m_h, m_x + kx2, m_v + kv2);

        // we should keep m_t within bounds, even though it would take a long time
        // to overrun the size of t_CKFLOAT aka double...
        m_t = m_t + m_h;
        m_x = m_x + m_h * (kx0 + 2*kx1 + 2*kx2 + kx3) / 6;
        m_v = m_v + m_h * (kv0 + 2*kv1 + 2*kv2 + kv3) / 6;

        return m_x;
    }

    t_CKFLOAT setAlpha( t_CKFLOAT p )
    {
        m_alpha = p;
        return p;
    }

    t_CKFLOAT setBeta( t_CKFLOAT p )
    {
        m_beta = p;
        return p;
    }

    t_CKFLOAT setDelta( t_CKFLOAT p )
    {
        m_delta = p;
        return p;
    }

    t_CKFLOAT setGamma( t_CKFLOAT p )
    {
        m_gamma = p;
        return p;
    }

    t_CKFLOAT setOmega( t_CKFLOAT p )
    {
        m_omega = p;
        return p;
    }

    t_CKFLOAT setStep( t_CKFLOAT p )
    {
        m_h = p;
        return p;
    }

    t_CKFLOAT setX( t_CKFLOAT p )
    {
        m_x = p;
        reset();
        return p;
    }

    t_CKFLOAT setV( t_CKFLOAT p )
    {
        m_v = p;
        reset();
        return p;
    }

    t_CKFLOAT getAlpha() { return m_alpha; }
    t_CKFLOAT getBeta() { return m_beta; }
    t_CKFLOAT getDelta() { return m_delta; }
    t_CKFLOAT getGamma() { return m_gamma; }
    t_CKFLOAT getOmega() { return m_omega; }
    t_CKFLOAT getStep() { return m_h; }
    t_CKFLOAT getX() { return m_x; }
    t_CKFLOAT getV() { return m_v; }

    t_CKVOID reset()
    {
        m_t = 0;
        return;
    }
    
private:
    // instance data
    t_CKFLOAT m_alpha;
    t_CKFLOAT m_beta;
    t_CKFLOAT m_delta;
    t_CKFLOAT m_gamma;
    t_CKFLOAT m_omega;

    t_CKFLOAT m_t;
    t_CKFLOAT m_x;
    t_CKFLOAT m_v;

    t_CKFLOAT m_h;

    t_CKFLOAT f2( t_CKFLOAT t,  t_CKFLOAT x,  t_CKFLOAT v )
    {
        // the Duffing equation:
        // x'' + delta x' + alpha x + beta x^3 = gamma cos (omega t)
        // decomposes to two first-order differential equations by setting v = x':
        // x' = v   = f1
        // v' = x'' = f2 = gamma cos (omega t) - delta v - alpha x - beta x^3
        return m_gamma * cos(m_omega * t) - m_delta * v - m_alpha * x - m_beta * pow(x, 3);
    }
};


// query function: chuck calls this when loading the Chugin
CK_DLL_QUERY( Duffing )
{
    QUERY->setname(QUERY, "Duffing");
    
    // begin the class definition
    QUERY->begin_class(QUERY, "Duffing", "UGen");
    QUERY->doc_class(QUERY, "An attempt at synthesis using the Duffing equation and RK4. ");

    QUERY->add_ctor(QUERY, duffing_ctor);
    QUERY->add_dtor(QUERY, duffing_dtor);
    
    QUERY->add_ugen_func(QUERY, duffing_tick, NULL, 1, 1);
    
    QUERY->add_mfun(QUERY, duffing_setAlpha, "float", "alpha");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "Stiffness. ");

    QUERY->add_mfun(QUERY, duffing_getAlpha, "float", "alpha");
    QUERY->doc_func(QUERY, "Stiffness. ");

    QUERY->add_mfun(QUERY, duffing_setAlpha, "float", "stiffness");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "Stiffness. ");

    QUERY->add_mfun(QUERY, duffing_getAlpha, "float", "stiffness");
    QUERY->doc_func(QUERY, "Stiffness. ");

    QUERY->add_mfun(QUERY, duffing_setBeta, "float", "beta");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "Non-linearity. ");

    QUERY->add_mfun(QUERY, duffing_getBeta, "float", "beta");
    QUERY->doc_func(QUERY, "Non-linearity. ");

    QUERY->add_mfun(QUERY, duffing_setBeta, "float", "nonlinearity");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "Non-linearity. ");

    QUERY->add_mfun(QUERY, duffing_getBeta, "float", "nonlinearity");
    QUERY->doc_func(QUERY, "Non-linearity. ");

    QUERY->add_mfun(QUERY, duffing_setDelta, "float", "delta");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "Damping. ");

    QUERY->add_mfun(QUERY, duffing_getDelta, "float", "delta");
    QUERY->doc_func(QUERY, "Damping. ");

    QUERY->add_mfun(QUERY, duffing_setDelta, "float", "damping");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "Damping. ");

    QUERY->add_mfun(QUERY, duffing_getDelta, "float", "damping");
    QUERY->doc_func(QUERY, "Damping. ");

    QUERY->add_mfun(QUERY, duffing_setGamma, "float", "gamma");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "Amplitude of driving force. ");

    QUERY->add_mfun(QUERY, duffing_getGamma, "float", "gamma");
    QUERY->doc_func(QUERY, "Amplitude of driving force. ");

    QUERY->add_mfun(QUERY, duffing_setGamma, "float", "drive");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "Amplitude of driving force. ");

    QUERY->add_mfun(QUERY, duffing_getGamma, "float", "drive");
    QUERY->doc_func(QUERY, "Amplitude of driving force. ");

    QUERY->add_mfun(QUERY, duffing_setOmega, "float", "omega");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "Angular frequency of driving force. ");

    QUERY->add_mfun(QUERY, duffing_getOmega, "float", "omega");
    QUERY->doc_func(QUERY, "Angular frequency of driving force. ");

    QUERY->add_mfun(QUERY, duffing_setOmega, "float", "freq");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "Angular frequency of driving force. ");

    QUERY->add_mfun(QUERY, duffing_getOmega, "float", "freq");
    QUERY->doc_func(QUERY, "Angular frequency of driving force. ");

    QUERY->add_mfun(QUERY, duffing_setStep, "float", "h");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "Step size. ");

    QUERY->add_mfun(QUERY, duffing_getStep, "float", "h");
    QUERY->doc_func(QUERY, "Step size. ");

    QUERY->add_mfun(QUERY, duffing_setStep, "float", "step");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "Step size. ");

    QUERY->add_mfun(QUERY, duffing_getStep, "float", "step");
    QUERY->doc_func(QUERY, "Step size. ");

    QUERY->add_mfun(QUERY, duffing_setX, "float", "x");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "Initial position. ");

    QUERY->add_mfun(QUERY, duffing_getX, "float", "x");
    QUERY->doc_func(QUERY, "Initial position. ");

    QUERY->add_mfun(QUERY, duffing_setV, "float", "v");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "Initial velocity. ");

    QUERY->add_mfun(QUERY, duffing_getV, "float", "v");
    QUERY->doc_func(QUERY, "Initial velocity. ");

    QUERY->add_mfun(QUERY, duffing_reset, "void", "reset");
    QUERY->doc_func(QUERY, "Reset time to zero. ");

    duffing_data_offset = QUERY->add_mvar(QUERY, "int", "@d_data", false);

    // end the class definition
    QUERY->end_class(QUERY);

    return TRUE;
}


// implementation for the constructor
CK_DLL_CTOR(duffing_ctor)
{
    // get the offset where we'll store our internal c++ class pointer
    OBJ_MEMBER_INT(SELF, duffing_data_offset) = 0;
    
    // instantiate our internal c++ class representation
    Duffing * d_obj = new Duffing(API->vm->get_srate(API, SHRED));
    
    // store the pointer in the ChucK object member
    OBJ_MEMBER_INT(SELF, duffing_data_offset) = (t_CKINT) d_obj;
}


// implementation for the destructor
CK_DLL_DTOR(duffing_dtor)
{
    // get our c++ class pointer
    Duffing * d_obj = (Duffing *) OBJ_MEMBER_INT(SELF, duffing_data_offset);
    // check it
    if( d_obj )
    {
        // clean up
        delete d_obj;
        OBJ_MEMBER_INT(SELF, duffing_data_offset) = 0;
        d_obj = NULL;
    }
}


// implementation for tick function
CK_DLL_TICK(duffing_tick)
{
    Duffing * d_obj = (Duffing *) OBJ_MEMBER_INT(SELF, duffing_data_offset);
 
    // invoke our tick function; store in the magical out variable
    if(d_obj) *out = d_obj->tick(in);

    return TRUE;
}


CK_DLL_MFUN(duffing_setAlpha)
{
    Duffing * d_obj = (Duffing *) OBJ_MEMBER_INT(SELF, duffing_data_offset);
    RETURN->v_float = d_obj->setAlpha(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(duffing_getAlpha)
{
    Duffing * d_obj = (Duffing *) OBJ_MEMBER_INT(SELF, duffing_data_offset);
    RETURN->v_float = d_obj->getAlpha();
}

CK_DLL_MFUN(duffing_setBeta)
{
    Duffing * d_obj = (Duffing *) OBJ_MEMBER_INT(SELF, duffing_data_offset);
    RETURN->v_float = d_obj->setBeta(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(duffing_getBeta)
{
    Duffing * d_obj = (Duffing *) OBJ_MEMBER_INT(SELF, duffing_data_offset);
    RETURN->v_float = d_obj->getBeta();
}

CK_DLL_MFUN(duffing_setDelta)
{
    Duffing * d_obj = (Duffing *) OBJ_MEMBER_INT(SELF, duffing_data_offset);
    RETURN->v_float = d_obj->setDelta(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(duffing_getDelta)
{
    Duffing * d_obj = (Duffing *) OBJ_MEMBER_INT(SELF, duffing_data_offset);
    RETURN->v_float = d_obj->getDelta();
}

CK_DLL_MFUN(duffing_setGamma)
{
    Duffing * d_obj = (Duffing *) OBJ_MEMBER_INT(SELF, duffing_data_offset);
    RETURN->v_float = d_obj->setGamma(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(duffing_getGamma)
{
    Duffing * d_obj = (Duffing *) OBJ_MEMBER_INT(SELF, duffing_data_offset);
    RETURN->v_float = d_obj->getGamma();
}

CK_DLL_MFUN(duffing_setOmega)
{
    Duffing * d_obj = (Duffing *) OBJ_MEMBER_INT(SELF, duffing_data_offset);
    RETURN->v_float = d_obj->setOmega(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(duffing_getOmega)
{
    Duffing * d_obj = (Duffing *) OBJ_MEMBER_INT(SELF, duffing_data_offset);
    RETURN->v_float = d_obj->getOmega();
}

CK_DLL_MFUN(duffing_setStep)
{
    Duffing * d_obj = (Duffing *) OBJ_MEMBER_INT(SELF, duffing_data_offset);
    RETURN->v_float = d_obj->setStep(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(duffing_getStep)
{
    Duffing * d_obj = (Duffing *) OBJ_MEMBER_INT(SELF, duffing_data_offset);
    RETURN->v_float = d_obj->getStep();
}

CK_DLL_MFUN(duffing_setX)
{
    Duffing * d_obj = (Duffing *) OBJ_MEMBER_INT(SELF, duffing_data_offset);
    RETURN->v_float = d_obj->setX(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(duffing_getX)
{
    Duffing * d_obj = (Duffing *) OBJ_MEMBER_INT(SELF, duffing_data_offset);
    RETURN->v_float = d_obj->getX();
}

CK_DLL_MFUN(duffing_setV)
{
    Duffing * d_obj = (Duffing *) OBJ_MEMBER_INT(SELF, duffing_data_offset);
    RETURN->v_float = d_obj->setV(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(duffing_getV)
{
    Duffing * d_obj = (Duffing *) OBJ_MEMBER_INT(SELF, duffing_data_offset);
    RETURN->v_float = d_obj->getV();
}

CK_DLL_MFUN(duffing_reset)
{
    Duffing * d_obj = (Duffing *) OBJ_MEMBER_INT(SELF, duffing_data_offset);
    d_obj->reset();
}
