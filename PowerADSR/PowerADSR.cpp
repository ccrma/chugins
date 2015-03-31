
/*-----------------------------------------------------------------------------
 Entaro!

 ChucK PowerADSR extends Envelope.

 An ADSR Envelope that allows for power-shaped envelope phases.

 Copyright (c) 2015 Eric Heep. All rights reserved.

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 U.S.A.
-----------------------------------------------------------------------------*/

#include "chuck_dl.h"
#include "chuck_def.h"

#include <stdio.h>
#include <limits.h>
#include <math.h>

CK_DLL_CTOR(poweradsr_ctor);
CK_DLL_DTOR(poweradsr_dtor);

CK_DLL_MFUN(poweradsr_setKeyOn);
CK_DLL_MFUN(poweradsr_setKeyOff);

CK_DLL_MFUN(poweradsr_setAll);
CK_DLL_MFUN(poweradsr_setAttack);
CK_DLL_MFUN(poweradsr_setDecay);
CK_DLL_MFUN(poweradsr_setSustain);
CK_DLL_MFUN(poweradsr_setRelease);

CK_DLL_MFUN(poweradsr_setAllCurves);
CK_DLL_MFUN(poweradsr_setAttackCurve);
CK_DLL_MFUN(poweradsr_setDecayCurve);
CK_DLL_MFUN(poweradsr_setReleaseCurve);

CK_DLL_TICK(poweradsr_tick);
t_CKINT poweradsr_data_offset = 0;

class PowerADSR
{
public:
    // constructor
    PowerADSR( t_CKFLOAT fs)
    {
        m_keyOn = 0;
        m_keyOff = 0;

        // stage counters
        m_attackCount = 0;
        m_decayCount = 0;
        m_releaseCount = 0;

        // stage durations
        m_attackDuration = 0.0;
        m_decayDuration = 0.0;
        m_releaseDuration = 0.0;

        // levels
        m_sustainLevel = 1.0;
        m_currentLevel = 0.0;
        m_keyOnLevel = 0.0;
        m_keyOffLevel = 0.0;

        // initializes to linear envelopes 
        m_attackCurve = 1.0;
        m_decayCurve = 1.0;
        m_releaseCurve = 1.0;
    }

    // for Chugins extending UGen
    SAMPLE tick( SAMPLE in )
    {
        if(m_keyOn == 1) 
        {
            if (m_attackCount < m_attackDuration) {
                m_attackCount++;
                m_currentLevel = pow(m_attackCount/m_attackDuration, m_attackCurve) * (1.0 - m_keyOnLevel) + m_keyOnLevel;

                return in * m_currentLevel;
            }
            if (m_decayCount < m_decayDuration) {
                m_decayCount++; 
                m_currentLevel = (1.0 - pow(m_decayCount/m_decayDuration, m_decayCurve)) * (1.0 - m_sustainLevel) + m_sustainLevel;

                return in * m_currentLevel;
            }
            else {
                return in * m_currentLevel;
            }
        }

        if(m_keyOff == 1) 
        {
            if (m_releaseCount < m_releaseDuration) {
                m_releaseCount++;
                m_currentLevel = ((1.0 - pow(m_releaseCount/m_releaseDuration, m_releaseCurve)) * m_keyOffLevel); 

                return in * m_currentLevel; 
            }
            else {
                m_keyOff = 0;
            }
        }
        return 0.0;
    }

    // keyOn stuff
    int setKeyOn()
    {
        m_keyOnLevel = m_currentLevel;

        m_attackCount = 0;
        m_decayCount = 0;
        m_keyOn = 1;
        m_keyOff = 0;

        return m_keyOn;
    }

    int setKeyOn( t_CKINT )
    {
        m_keyOnLevel = m_currentLevel;

        m_attackCount = 0;
        m_decayCount = 0;
        m_keyOn = 1;
        m_keyOff = 0;

        return m_keyOn;
    }

    // keyOff stuff
    int setKeyOff() 
    {
        m_keyOffLevel = m_currentLevel;
        m_releaseCount = 0;

        m_keyOn = 0;
        m_keyOff = 1;

        return m_keyOff;
    }

    int setKeyOff( t_CKINT )
    {
        m_keyOffLevel = m_currentLevel;
        m_releaseCount = 0;

        m_keyOn = 0;
        m_keyOff = 1;

        return m_keyOn;
    }

    // set all durations
    void setAll( t_CKDUR a, t_CKDUR d, t_CKFLOAT s, t_CKDUR r ) {
        setAttack(a);
        setDecay(d);
        setSustain(s);
        setRelease(r);
    }

    float setAttack( t_CKDUR a ) 
    {
        m_attackDuration = (float)a;

        m_keyOnLevel = m_currentLevel;
        m_attackCount = 0;

        return a;
    }

    float setDecay( t_CKDUR d ) 
    {
        m_decayDuration = (float)d;

        m_sustainLevel = m_currentLevel;
        m_decayCount = 0;
        
        return d;
    }

    float setSustain( t_CKFLOAT s ) 
    {
        m_sustainLevel = s;
        return s;
    }

    float setRelease( t_CKDUR r ) 
    {
        m_releaseDuration = (float)r;

        m_keyOffLevel = m_currentLevel;
        m_releaseCount = 0;

        return r;
    }

    // set all power curves
    void setAllCurves( t_CKFLOAT a, t_CKFLOAT d, t_CKFLOAT r ) {
        setAttackCurve(a);
        setDecayCurve(d);
        setReleaseCurve(r);
    }

    float setAttackCurve( t_CKFLOAT ac) 
    {
        m_attackCurve = ac;

        m_keyOnLevel = m_currentLevel;
        m_attackCount = 0;

        return ac;
    }

    float setDecayCurve( t_CKFLOAT dc) 
    {
        m_decayCurve = dc;

        m_sustainLevel = m_currentLevel;
        m_decayCount = 0;

        return dc;
    }

    float setReleaseCurve( t_CKFLOAT rc) 
    {
        m_releaseCurve = rc;

        m_keyOffLevel = m_currentLevel;
        m_releaseCount = 0;

        return rc;
    }

private:
    int m_keyOn;
    int m_keyOff;

    // sample counters 
    int m_attackCount;
    int m_decayCount;
    int m_releaseCount;

    // durations
    float m_attackDuration;
    float m_decayDuration;
    float m_releaseDuration;

    // levels
    float m_sustainLevel;
    float m_currentLevel;
    float m_keyOnLevel;
    float m_keyOffLevel;

    // curves
    float m_attackCurve;
    float m_decayCurve;
    float m_releaseCurve;
};


CK_DLL_QUERY( PowerADSR )
{
    QUERY->setname(QUERY, "PowerADSR");
    
    QUERY->begin_class(QUERY, "PowerADSR", "Envelope");
    QUERY->doc_class(QUERY, "ADSR envelope that uses a power function to create curved envelope phases, extends Envelope.");
    
    QUERY->add_ctor(QUERY, poweradsr_ctor);
    QUERY->add_dtor(QUERY, poweradsr_dtor);
    
    QUERY->add_ugen_func(QUERY, poweradsr_tick, NULL, 1, 1);

    QUERY->add_mfun(QUERY, poweradsr_setKeyOn, "int", "keyOn");
    QUERY->doc_func(QUERY, "Begins the attack phase of the envelope. ");

    QUERY->add_mfun(QUERY, poweradsr_setKeyOn, "int", "keyOn");
    QUERY->add_arg(QUERY, "int", "keyOn");
    QUERY->doc_func(QUERY, "Begins the attack phase of the envelope. ");

    QUERY->add_mfun(QUERY, poweradsr_setKeyOff, "int", "keyOff");
    QUERY->doc_func(QUERY, "Begins the release phase of the envelope. ");

    QUERY->add_mfun(QUERY, poweradsr_setKeyOff, "int", "keyOff");
    QUERY->add_arg(QUERY, "int", "keyOff");
    QUERY->doc_func(QUERY, "Begins the release phase of the envelope. ");
    
    QUERY->add_mfun(QUERY, poweradsr_setAll, "void", "set");
    QUERY->add_arg(QUERY, "dur", "attackDuration");
    QUERY->add_arg(QUERY, "dur", "decayDuration");
    QUERY->add_arg(QUERY, "float", "sustainLevel");
    QUERY->add_arg(QUERY, "dur", "releaseDuration");
    QUERY->doc_func(QUERY, "Sets duration of the attack, decay, and release phases; as well as the sustain level (ADSR order). ");

    QUERY->add_mfun(QUERY, poweradsr_setAttack, "dur", "attack");
    QUERY->add_arg(QUERY, "dur", "attackDuration");
    QUERY->doc_func(QUERY, "Sets duration of the attack phase. ");

    QUERY->add_mfun(QUERY, poweradsr_setDecay, "dur", "decay");
    QUERY->add_arg(QUERY, "dur", "decayDuration");
    QUERY->doc_func(QUERY, "Sets duration of the decay phase. ");
 
    QUERY->add_mfun(QUERY, poweradsr_setSustain, "float", "sustain");
    QUERY->add_arg(QUERY, "float", "sustainLevel");
    QUERY->doc_func(QUERY, "Sets sustain level. ");

    QUERY->add_mfun(QUERY, poweradsr_setRelease, "dur", "release");
    QUERY->add_arg(QUERY, "dur", "releaseDuration");
    QUERY->doc_func(QUERY, "Sets duration of the release phase. ");
 
    QUERY->add_mfun(QUERY, poweradsr_setAllCurves, "void", "setCurves");
    QUERY->add_arg(QUERY, "float", "attackCurve");
    QUERY->add_arg(QUERY, "float", "decayCurve");
    QUERY->add_arg(QUERY, "float", "releaseCurve");
    QUERY->doc_func(QUERY, "Sets envelope curves of the attack, decay, and release phases. ");

    QUERY->add_mfun(QUERY, poweradsr_setAttackCurve, "float", "attackCurve");
    QUERY->add_arg(QUERY, "float", "attackCurve");
    QUERY->doc_func(QUERY, "Sets envelope curve of the attack phase. ");

    QUERY->add_mfun(QUERY, poweradsr_setDecayCurve, "float", "decayCurve");
    QUERY->add_arg(QUERY, "float", "decayCurve");
    QUERY->doc_func(QUERY, "Sets envelope curve of the decay phase. ");

    QUERY->add_mfun(QUERY, poweradsr_setReleaseCurve, "float", "releaseCurve");
    QUERY->add_arg(QUERY, "float", "releaseCurve");
    QUERY->doc_func(QUERY, "Sets envelope curve of the release phase. ");

    // this reserves a variable in the ChucK internal class to store 
    // referene to the c++ class we defined above
    poweradsr_data_offset = QUERY->add_mvar(QUERY, "int", "@ee_data", false);

    // end the class definition
    // IMPORTANT: this MUST be called!
    QUERY->end_class(QUERY);

    // wasn't that a breeze?
    return TRUE;
}


// implementation for the constructor
CK_DLL_CTOR(poweradsr_ctor)
{
    // get the offset where we'll store our internal c++ class pointer
    OBJ_MEMBER_INT(SELF, poweradsr_data_offset) = 0;
    
    // instantiate our internal c++ class representation
    PowerADSR * bcdata = new PowerADSR(API->vm->get_srate());
    
    // store the pointer in the ChucK object member
    OBJ_MEMBER_INT(SELF, poweradsr_data_offset) = (t_CKINT) bcdata;
}


// implementation for the destructor
CK_DLL_DTOR(poweradsr_dtor)
{
    // get our c++ class pointer
    PowerADSR * bcdata = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    // check it
    if( bcdata )
    {
        // clean up
        delete bcdata;
        OBJ_MEMBER_INT(SELF, poweradsr_data_offset) = 0;
        bcdata = NULL;
    }
}


// implementation for tick function
CK_DLL_TICK(poweradsr_tick)
{
    // get our c++ class pointer
    PowerADSR * c = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
 
    // invoke our tick function; store in the magical out variable
    if(c) *out = c->tick(in);

    // yes
    return TRUE;
}

CK_DLL_MFUN(poweradsr_setKeyOn)
{
    PowerADSR * bcdata = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    bcdata->setKeyOn(1);
    RETURN->v_int = 1;
}

CK_DLL_MFUN(poweradsr_setKeyOff)
{
    PowerADSR * bcdata = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    bcdata->setKeyOff(1);
    RETURN->v_int = 1;
}

CK_DLL_MFUN(poweradsr_setAttack)
{
    PowerADSR * bcdata = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    RETURN->v_dur = bcdata->setAttack(GET_NEXT_DUR(ARGS));
}

CK_DLL_MFUN(poweradsr_setDecay)
{
    PowerADSR * bcdata = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    RETURN->v_dur = bcdata->setDecay(GET_NEXT_DUR(ARGS));
}

CK_DLL_MFUN(poweradsr_setAll)
{
    PowerADSR * bcdata = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    t_CKDUR aDur = GET_NEXT_DUR(ARGS);
    t_CKDUR dDur = GET_NEXT_DUR(ARGS);
    t_CKFLOAT sLvl = GET_NEXT_FLOAT(ARGS);
    t_CKDUR rDur = GET_NEXT_DUR(ARGS);
    bcdata->setAll(aDur, dDur, sLvl, rDur);
}

CK_DLL_MFUN(poweradsr_setSustain)
{
    PowerADSR * bcdata = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    RETURN->v_float = bcdata->setSustain(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(poweradsr_setRelease)
{
    PowerADSR * bcdata = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    RETURN->v_dur = bcdata->setRelease(GET_NEXT_DUR(ARGS));
}

CK_DLL_MFUN(poweradsr_setAllCurves)
{
    PowerADSR * bcdata = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    t_CKFLOAT aCurv = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT dCurv = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT rCurv = GET_NEXT_FLOAT(ARGS);
    bcdata->setAllCurves(aCurv, dCurv, rCurv);
}

CK_DLL_MFUN(poweradsr_setAttackCurve)
{
    PowerADSR * bcdata = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    RETURN->v_float = bcdata->setAttackCurve(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(poweradsr_setDecayCurve)
{
    PowerADSR * bcdata = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    RETURN->v_float = bcdata->setDecayCurve(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(poweradsr_setReleaseCurve)
{
    PowerADSR * bcdata = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    RETURN->v_float = bcdata->setReleaseCurve(GET_NEXT_FLOAT(ARGS));
}

