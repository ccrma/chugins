/*-----------------------------------------------------------------------------
 Entaro!

 An ADSR Envelope that allows for power-shaped stages.

 Copyright (c) 2017 Eric Heep. All rights reserved.

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

// setters
CK_DLL_MFUN(poweradsr_setKeyOn);
CK_DLL_MFUN(poweradsr_setKeyOff);

CK_DLL_MFUN(poweradsr_setAllTimes);
CK_DLL_MFUN(poweradsr_setAttackTime);
CK_DLL_MFUN(poweradsr_setDecayTime);
CK_DLL_MFUN(poweradsr_setReleaseTime);

CK_DLL_MFUN(poweradsr_setAllCurves);
CK_DLL_MFUN(poweradsr_setAttackCurve);
CK_DLL_MFUN(poweradsr_setDecayCurve);
CK_DLL_MFUN(poweradsr_setReleaseCurve);

CK_DLL_MFUN(poweradsr_setSustainLevel);

// getters
CK_DLL_MFUN(poweradsr_getSustainLevel);

CK_DLL_MFUN(poweradsr_getAttackTime);
CK_DLL_MFUN(poweradsr_getDecayTime);
CK_DLL_MFUN(poweradsr_getReleaseTime);

CK_DLL_MFUN(poweradsr_getAttackCurve);
CK_DLL_MFUN(poweradsr_getDecayCurve);
CK_DLL_MFUN(poweradsr_getReleaseCurve);

CK_DLL_MFUN(poweradsr_getValue);
CK_DLL_MFUN(poweradsr_getState);

// tick
CK_DLL_TICK(poweradsr_tick);

t_CKINT poweradsr_data_offset = 0;


class PowerADSR
{
public:
    // constructor
    PowerADSR( t_CKFLOAT fs)
    {
        // stage durations, set to 1::second
        m_attackDuration = fs;
        m_decayDuration = fs;
        m_releaseDuration = fs;

        m_sustainLevel = 0.5;

        // for one less calculation per tick
        m_inverseAttackDuration = 1.0/fs;
        m_inverseDecayDuration = 1.0/fs;
        m_inverseReleaseDuration = 1.0/fs;

        // values
        m_linearValue = 0.0;
        m_powerValue = 0.0;
        m_offsetValue = 0.0;
        m_scalarValue = 1.0;

        // sample multipler
        m_value = 0.0;
        m_target = 1.0;

        // time tracking
        m_sampleCount = 0.0;

        // initializes to linear envelopes
        m_attackCurve = 1.0;
        m_decayCurve = 1.0;
        m_releaseCurve = 1.0;

        m_curve = 1.0;
        m_inverseDuration = 0.0;

        m_state = DONE;
    }

    // for Chugins extending UGen
    SAMPLE tick( SAMPLE in )
    {
        switch(m_state)
        {

        case ATTACK:
            m_sampleCount++;
            if (m_sampleCount >= m_attackDuration) {
                m_sampleCount = m_decayDuration;
                m_inverseDuration = m_inverseDecayDuration;
                m_curve = m_decayCurve;
                m_offsetValue = m_sustainLevel;
                m_scalarValue = m_target - m_sustainLevel;

                m_state = DECAY;
            }
            break;

        case DECAY:
            m_sampleCount--;
            if (m_sampleCount <= 0.0) {
                m_sampleCount = m_releaseDuration;
                m_inverseDuration = m_inverseReleaseDuration;
                m_curve = m_releaseCurve;
                m_offsetValue = 0.0;
                m_scalarValue = m_sustainLevel;

                m_state = SUSTAIN;
            }
            break;

        case SUSTAIN:
            break;

        case RELEASE:
            m_sampleCount--;
            if (m_sampleCount <= 0.0) {
                m_linearValue = 0.0;
                m_scalarValue = 1.0;
                m_state = DONE;
            }
            break;

        case DONE:
            break;
        }

        // linear scaling derived from duration
        m_linearValue = m_sampleCount * m_inverseDuration;

        // power curve
        m_powerValue = pow(m_linearValue, m_curve);

        // added calculations for stage changes
        m_value = m_powerValue * m_scalarValue + m_offsetValue;

        return in * m_value;
    }

    // keyOn stuff
    int setKeyOn()
    {
        m_sampleCount = 0.0;
        m_inverseDuration = m_inverseAttackDuration;
        m_curve = m_attackCurve;
        m_offsetValue = m_value;
        m_scalarValue = m_target - m_value;

        m_state = ATTACK;
        return 1;
    }

    int setKeyOn( t_CKINT )
    {
        m_sampleCount = 0.0;
        m_inverseDuration = m_inverseAttackDuration;
        m_curve = m_attackCurve;
        m_offsetValue = m_value;
        m_scalarValue = m_target - m_value;

        m_state = ATTACK;
        return 1;
    }

    // keyOff stuff
    int setKeyOff()
    {
        m_sampleCount = m_releaseDuration;
        m_inverseDuration = m_inverseReleaseDuration;
        m_curve = m_releaseCurve;
        m_offsetValue = 0.0;
        m_scalarValue = m_value;

        m_state = RELEASE;
        return 0;
    }

    int setKeyOff( t_CKINT )
    {
        m_sampleCount = m_releaseDuration;
        m_inverseDuration = m_inverseReleaseDuration;
        m_curve = m_releaseCurve;
        m_offsetValue = 0.0;
        m_scalarValue = m_value;

        m_state = RELEASE;
        return 0;
    }

    // set all durations and sustain level
    void setAll( t_CKDUR a, t_CKDUR d, t_CKFLOAT s, t_CKDUR r )
    {
        setAttackTime(a);
        setDecayTime(d);
        setSustainLevel(s);
        setReleaseTime(r);
    }

    float setAttackTime( t_CKDUR a )
    {
        m_attackDuration = (float)a;

        if (m_attackDuration < 0.0) {
            m_attackDuration = -m_attackDuration;
        }

        m_inverseAttackDuration = 1.0 / m_attackDuration;
        return a;
    }

    float setDecayTime( t_CKDUR d )
    {
        m_decayDuration = (float)d;

        if (m_decayDuration < 0.0) {
            m_decayDuration = -m_decayDuration;
        }

        m_inverseDecayDuration = 1.0 / m_decayDuration;
        return d;
    }

    float setSustainLevel( t_CKFLOAT s )
    {
        m_sustainLevel = s;

        if (m_sustainLevel < 0.0) {
            m_sustainLevel = -m_sustainLevel;
        }

        return s;
    }

    float setReleaseTime( t_CKDUR r )
    {
        m_releaseDuration = (float)r;

        if (m_releaseDuration < 0.0) {
            m_releaseDuration = -m_releaseDuration;
        }

        m_inverseReleaseDuration = 1.0 / m_releaseDuration;
        return m_releaseDuration;
    }

    // set all power curves
    void setAllCurves( t_CKFLOAT a, t_CKFLOAT d, t_CKFLOAT r )
    {
        setAttackCurve(a);
        setDecayCurve(d);
        setReleaseCurve(r);
    }

    float setAttackCurve( t_CKFLOAT a)
    {
        m_attackCurve = a;
        return a;
    }

    float setDecayCurve( t_CKFLOAT d)
    {
        m_decayCurve = d;
        return d;
    }

    float setReleaseCurve( t_CKFLOAT r)
    {
        m_releaseCurve = r;
        return r;
    }

    float getAttackTime()
    {
        return m_attackDuration;
    }

    float getDecayTime()
    {
        return m_decayDuration;
    }

    float getSustainLevel()
    {
        return m_sustainLevel;
    }

    float getReleaseTime()
    {
        return m_releaseDuration;
    }

    float getAttackCurve()
    {
        return m_attackCurve;
    }

    float getDecayCurve()
    {
        return m_decayCurve;
    }

    float getReleaseCurve()
    {
        return m_releaseCurve;
    }

    float getValue()
    {
        return m_value;
    }

    int getState()
    {
        return m_state;
    }


private:
    // stage durations
    float m_attackDuration;
    float m_decayDuration;
    float m_releaseDuration;

    float m_sustainLevel;

    // for one less calculation per tick
    float m_inverseAttackDuration;
    float m_inverseDecayDuration;
    float m_inverseReleaseDuration;

    // values
    float m_linearValue;
    float m_powerValue;
    float m_offsetValue;
    float m_scalarValue;

    // sample multiplier
    float m_value;
    float m_target;

    // time tracking
    float m_sampleCount;

    // curves
    float m_attackCurve;
    float m_decayCurve;
    float m_releaseCurve;

    float m_curve;
    float m_inverseDuration;

    enum States {
        DONE = 0,
        ATTACK,
        DECAY,
        SUSTAIN,
        RELEASE,
    };

    States m_state;
};


CK_DLL_QUERY( PowerADSR)
{
    QUERY->setname(QUERY, "PowerADSR");

    QUERY->begin_class(QUERY, "PowerADSR", "Envelope");
    QUERY->doc_class(QUERY, "ADSR envelope that uses a power function to create curved envelope phases. In general, curves under 1.0 are sharp, while curves over 1.0 are soft.");
    QUERY->add_ex(QUERY, "effects/PowerADSR-feedback-beatings.ck");
    QUERY->add_ex(QUERY, "effects/PowerADSR.ck");

    QUERY->add_ctor(QUERY, poweradsr_ctor);
    QUERY->add_dtor(QUERY, poweradsr_dtor);

    QUERY->add_ugen_func(QUERY, poweradsr_tick, NULL, 1, 1);

    QUERY->add_mfun(QUERY, poweradsr_setKeyOn, "int", "keyOn");
    QUERY->doc_func(QUERY, "Begins the attack phase of the envelope.");

    QUERY->add_mfun(QUERY, poweradsr_setKeyOn, "int", "keyOn");
    QUERY->add_arg(QUERY, "int", "keyOn");
    QUERY->doc_func(QUERY, "Begins the attack phase of the envelope.");

    QUERY->add_mfun(QUERY, poweradsr_setKeyOff, "int", "keyOff");
    QUERY->doc_func(QUERY, "Begins the release phase of the envelope.");

    QUERY->add_mfun(QUERY, poweradsr_setKeyOff, "int", "keyOff");
    QUERY->add_arg(QUERY, "int", "keyOff");
    QUERY->doc_func(QUERY, "Begins the release phase of the envelope.");

    QUERY->add_mfun(QUERY, poweradsr_setAllTimes, "void", "set");
    QUERY->add_arg(QUERY, "dur", "attackDuration");
    QUERY->add_arg(QUERY, "dur", "decayDuration");
    QUERY->add_arg(QUERY, "float", "sustainLevel");
    QUERY->add_arg(QUERY, "dur", "releaseDuration");
    QUERY->doc_func(QUERY, "Sets duration of the attack, decay, and release phases; as well as the sustain level (ADSR order).");

    QUERY->add_mfun(QUERY, poweradsr_setAttackTime, "dur", "attack");
    QUERY->add_arg(QUERY, "dur", "attackDuration");
    QUERY->doc_func(QUERY, "Sets duration of the attack phase.");

    QUERY->add_mfun(QUERY, poweradsr_setAttackTime, "dur", "attackTime");
    QUERY->add_arg(QUERY, "dur", "attackDuration");
    QUERY->doc_func(QUERY, "Sets duration of the attack phase.");

    QUERY->add_mfun(QUERY, poweradsr_setDecayTime, "dur", "decay");
    QUERY->add_arg(QUERY, "dur", "decayDuration");
    QUERY->doc_func(QUERY, "Sets duration of the decay phase.");

    QUERY->add_mfun(QUERY, poweradsr_setDecayTime, "dur", "decayTime");
    QUERY->add_arg(QUERY, "dur", "decayDuration");
    QUERY->doc_func(QUERY, "Sets duration of the decay phase.");

    QUERY->add_mfun(QUERY, poweradsr_setSustainLevel, "float", "sustainLevel");
    QUERY->add_arg(QUERY, "float", "sustainLevel");
    QUERY->doc_func(QUERY, "Sets sustain level.");

    QUERY->add_mfun(QUERY, poweradsr_setReleaseTime, "dur", "release");
    QUERY->add_arg(QUERY, "dur", "releaseDuration");
    QUERY->doc_func(QUERY, "Sets duration of the release phase.");

    QUERY->add_mfun(QUERY, poweradsr_setReleaseTime, "dur", "releaseTime");
    QUERY->add_arg(QUERY, "dur", "releaseDuration");
    QUERY->doc_func(QUERY, "Sets duration of the release phase.");

    QUERY->add_mfun(QUERY, poweradsr_setAllCurves, "void", "setCurves");
    QUERY->add_arg(QUERY, "float", "attackCurve");
    QUERY->add_arg(QUERY, "float", "decayCurve");
    QUERY->add_arg(QUERY, "float", "releaseCurve");
    QUERY->doc_func(QUERY, "Sets envelope curves of the attack, decay, and release phases.");

    QUERY->add_mfun(QUERY, poweradsr_setAttackCurve, "float", "attackCurve");
    QUERY->add_arg(QUERY, "float", "attackCurve");
    QUERY->doc_func(QUERY, "Sets envelope curve of the attack phase.");

    QUERY->add_mfun(QUERY, poweradsr_setDecayCurve, "float", "decayCurve");
    QUERY->add_arg(QUERY, "float", "decayCurve");
    QUERY->doc_func(QUERY, "Sets envelope curve of the decay phase.");

    QUERY->add_mfun(QUERY, poweradsr_setReleaseCurve, "float", "releaseCurve");
    QUERY->add_arg(QUERY, "float", "releaseCurve");
    QUERY->doc_func(QUERY, "Sets envelope curve of the release phase.");

    QUERY->add_mfun(QUERY, poweradsr_getSustainLevel, "float", "sustainLevel");
    QUERY->doc_func(QUERY, "Gets sustain level.");

    QUERY->add_mfun(QUERY, poweradsr_getAttackTime, "dur", "attackTime");
    QUERY->doc_func(QUERY, "Gets the attack duration.");

    QUERY->add_mfun(QUERY, poweradsr_getDecayTime, "dur", "decayTime");
    QUERY->doc_func(QUERY, "Gets the decay duration.");

    QUERY->add_mfun(QUERY, poweradsr_getReleaseTime, "dur", "releaseTime");
    QUERY->doc_func(QUERY, "Gets the release duration.");

    QUERY->add_mfun(QUERY, poweradsr_getAttackCurve, "float", "attackCurve");
    QUERY->doc_func(QUERY, "Gets the attack curve.");

    QUERY->add_mfun(QUERY, poweradsr_getDecayCurve, "float", "decayCurve");
    QUERY->doc_func(QUERY, "Gets the decay curve.");

    QUERY->add_mfun(QUERY, poweradsr_getReleaseCurve, "float", "releaseCurve");
    QUERY->doc_func(QUERY, "Gets the release curve.");

    QUERY->add_mfun(QUERY, poweradsr_getValue, "float", "value");
    QUERY->doc_func(QUERY, "Gets current envelope value.");

    QUERY->add_mfun(QUERY, poweradsr_getState, "int", "state");
    QUERY->doc_func(QUERY, "Gets current state.");

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
    PowerADSR * padsr_obj = new PowerADSR(API->vm->get_srate(API, SHRED));

    // store the pointer in the ChucK object member
    OBJ_MEMBER_INT(SELF, poweradsr_data_offset) = (t_CKINT) padsr_obj;
}


// implementation for the destructor
CK_DLL_DTOR(poweradsr_dtor)
{
    // get our c++ class pointer
    PowerADSR * padsr_obj = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    // check it
    if( padsr_obj )
    {
        // clean up
        delete padsr_obj;
        OBJ_MEMBER_INT(SELF, poweradsr_data_offset) = 0;
        padsr_obj = NULL;
    }
}


// implementation for tick function
CK_DLL_TICK(poweradsr_tick)
{
    // get our c++ class pointer
    PowerADSR * padsr_obj = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);

    // invoke our tick function; store in the magical out variable
    if(padsr_obj) *out = padsr_obj->tick(in);

    // yes
    return TRUE;
}

CK_DLL_MFUN(poweradsr_setKeyOn)
{
    PowerADSR * padsr_obj = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    padsr_obj->setKeyOn(1);
    RETURN->v_int = 1;
}

CK_DLL_MFUN(poweradsr_setKeyOff)
{
    PowerADSR * padsr_obj = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    padsr_obj->setKeyOff(1);
    RETURN->v_int = 1;
}

CK_DLL_MFUN(poweradsr_setAttackTime)
{
    PowerADSR * padsr_obj = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    RETURN->v_dur = padsr_obj->setAttackTime(GET_NEXT_DUR(ARGS));
}

CK_DLL_MFUN(poweradsr_setDecayTime)
{
    PowerADSR * padsr_obj = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    RETURN->v_dur = padsr_obj->setDecayTime(GET_NEXT_DUR(ARGS));
}

CK_DLL_MFUN(poweradsr_setAllTimes)
{
    PowerADSR * padsr_obj = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    t_CKDUR aDur = GET_NEXT_DUR(ARGS);
    t_CKDUR dDur = GET_NEXT_DUR(ARGS);
    t_CKFLOAT sLvl = GET_NEXT_FLOAT(ARGS);
    t_CKDUR rDur = GET_NEXT_DUR(ARGS);
    padsr_obj->setAll(aDur, dDur, sLvl, rDur);
}

CK_DLL_MFUN(poweradsr_setSustainLevel)
{
    PowerADSR * padsr_obj = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    RETURN->v_float = padsr_obj->setSustainLevel(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(poweradsr_setReleaseTime)
{
    PowerADSR * padsr_obj = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    RETURN->v_dur = padsr_obj->setReleaseTime(GET_NEXT_DUR(ARGS));
}

CK_DLL_MFUN(poweradsr_setAllCurves)
{
    PowerADSR * padsr_obj = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    t_CKFLOAT aCurv = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT dCurv = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT rCurv = GET_NEXT_FLOAT(ARGS);
    padsr_obj->setAllCurves(aCurv, dCurv, rCurv);
}

CK_DLL_MFUN(poweradsr_setAttackCurve)
{
    PowerADSR * padsr_obj = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    RETURN->v_float = padsr_obj->setAttackCurve(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(poweradsr_setDecayCurve)
{
    PowerADSR * padsr_obj = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    RETURN->v_float = padsr_obj->setDecayCurve(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(poweradsr_setReleaseCurve)
{
    PowerADSR * padsr_obj = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    RETURN->v_float = padsr_obj->setReleaseCurve(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(poweradsr_getSustainLevel)
{
    PowerADSR * padsr_obj = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    RETURN->v_float = padsr_obj->getSustainLevel();
}

CK_DLL_MFUN(poweradsr_getAttackTime)
{
    PowerADSR * padsr_obj = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    RETURN->v_dur = padsr_obj->getAttackTime();
}

CK_DLL_MFUN(poweradsr_getDecayTime)
{
    PowerADSR * padsr_obj = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    RETURN->v_dur = padsr_obj->getDecayTime();
}

CK_DLL_MFUN(poweradsr_getReleaseTime)
{
    PowerADSR * padsr_obj = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    RETURN->v_dur = padsr_obj->getReleaseTime();
}

CK_DLL_MFUN(poweradsr_getAttackCurve)
{
    PowerADSR * padsr_obj = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    RETURN->v_float = padsr_obj->getAttackCurve();
}

CK_DLL_MFUN(poweradsr_getDecayCurve)
{
    PowerADSR * padsr_obj = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    RETURN->v_float = padsr_obj->getDecayCurve();
}

CK_DLL_MFUN(poweradsr_getReleaseCurve)
{
    PowerADSR * padsr_obj = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    RETURN->v_float = padsr_obj->getReleaseCurve();
}

CK_DLL_MFUN(poweradsr_getValue)
{
    PowerADSR * padsr_obj = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    RETURN->v_float = padsr_obj->getValue();
}

CK_DLL_MFUN(poweradsr_getState)
{
    PowerADSR * padsr_obj = (PowerADSR *) OBJ_MEMBER_INT(SELF, poweradsr_data_offset);
    RETURN->v_int= padsr_obj->getState();
}
