/*-----------------------------------------------------------------------------
 Entaro!

 ChucK WinFuncEnv extends Envelope.

 An AR Envelope that allows for envelopes in the shape of various window functions.
 http://en.wikipedia.org/wiki/Window_function

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


//-----------------------------------------------------------------------------

#include "chuck_dl.h"
#include "chuck_def.h"

#include <stdio.h>
#include <limits.h>
#ifdef WIN32
#define _USE_MATH_DEFINES
#endif
#include <math.h>

CK_DLL_CTOR(winfuncenv_ctor);
CK_DLL_DTOR(winfuncenv_dtor);

// ~ envelope functions
CK_DLL_MFUN(winfuncenv_setKeyOn);
CK_DLL_MFUN(winfuncenv_setKeyOff);
CK_DLL_MFUN(winfuncenv_setAttack);
CK_DLL_MFUN(winfuncenv_setRelease);
CK_DLL_MFUN(winfuncenv_getWindowValue);

// ~ window functions
CK_DLL_MFUN(winfuncenv_setBlackman);
CK_DLL_MFUN(winfuncenv_setBlackmanArg);
CK_DLL_MFUN(winfuncenv_setBlackmanDerivative);
CK_DLL_MFUN(winfuncenv_setBlackmanHarris);
CK_DLL_MFUN(winfuncenv_setBlackmanNutall);
CK_DLL_MFUN(winfuncenv_setExponential);
CK_DLL_MFUN(winfuncenv_setExponentialArg);
CK_DLL_MFUN(winfuncenv_setHann);
CK_DLL_MFUN(winfuncenv_setHannPoisson);
CK_DLL_MFUN(winfuncenv_setHannPoissonArg);
// TODO CK_DLL_MFUN(winfuncenv_setKaiser);
CK_DLL_MFUN(winfuncenv_setNutall);
CK_DLL_MFUN(winfuncenv_setParzen);
CK_DLL_MFUN(winfuncenv_setPoisson);
CK_DLL_MFUN(winfuncenv_setPoissonArg);
CK_DLL_MFUN(winfuncenv_setTukey);
CK_DLL_MFUN(winfuncenv_setTukeyArg);
CK_DLL_MFUN(winfuncenv_setWelch);

CK_DLL_TICK(winfuncenv_tick);
t_CKINT winfuncenv_data_offset = 0;

class WindowFunc
{
public:
    virtual float attackWindow (int n, int attack) = 0;
    virtual float releaseWindow (int n, int release) = 0;
};

class BlackmanFunc : public WindowFunc
{
public:
    BlackmanFunc(float a) {
        // blackman coefficients
        a0 = (1.0 - a)/2.0;
        a1 = 1.0/2.0;
        a2 = a/2.0;

        // delicious pies
        two_pi = 2.0 * M_PI;
        four_pi = 4.0 * M_PI;

        // optimization variables
        N_inv = 0.0;

        twoPiVal = 0.0;
        fourPiVal = 0.0;
    }

    float attackWindow (int n, int attack) {
        if( n == 0 ) {
            N_inv = 1.0/(attack * 2);

            twoPiVal = two_pi * N_inv;
            fourPiVal = four_pi * N_inv;

            return a0 - a1 + a2;
        }

        return a0 - a1 * cos(twoPiVal * n) + a2 * cos(fourPiVal * n);
    }
    float releaseWindow (int n, int release) {
        if( n == 0 ) {
            N_inv = 1.0/(release * 2);

            twoPiVal = two_pi * N_inv;
            fourPiVal = four_pi * N_inv;
        }

        n = n + release;

        return a0 - a1 * cos(twoPiVal * n) + a2 * cos(fourPiVal * n);
    }
private:
    // blackman coefficients
    float a0, a1, a2;

    // delicious pies
    float two_pi,   four_pi;
    float twoPiVal, fourPiVal;

    // optimization variables
    float N_inv;
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class BlackmanDerivativeFunc : public WindowFunc
{
public:
    BlackmanDerivativeFunc(float _a0, float _a1, float _a2, float _a3) {
        // blackman-derivative coefficients
        a0 = _a0;
        a1 = _a1;
        a2 = _a2;
        a3 = _a3;

        two_pi = 2.0 * M_PI;
        four_pi = 4.0 * M_PI;
        six_pi = 6.0 * M_PI;

        N_inv = 0.0;

        twoPiVal = 0.0;
        fourPiVal = 0.0;
        sixPiVal = 0.0;
    }

    float attackWindow (int n, int attack) {
        if( n == 0 ) {
            N_inv = 1.0/(attack * 2);

            twoPiVal = two_pi * N_inv;
            fourPiVal = four_pi * N_inv;
            sixPiVal = six_pi * N_inv;

            return a0 - a1 + a2 - a3;
        }

        return a0 - a1 * cos(twoPiVal * n) + a2 * cos(fourPiVal * n) - a3 * cos(sixPiVal * n);
    }

    float releaseWindow (int n, int release) {
        if( n == 0 ) {
            N_inv = 1.0/(release * 2);

            twoPiVal = two_pi * N_inv;
            fourPiVal = four_pi * N_inv;
            sixPiVal = six_pi * N_inv;
        }

        n = n + release;

        return a0 - a1 * cos(twoPiVal * n) + a2 * cos(fourPiVal * n) - a3 * cos(sixPiVal * n);
    }

private:
    // blackman-derivative coefficients
    float a0, a1, a2, a3;

    float two_pi,   four_pi,   six_pi;
    float twoPiVal, fourPiVal, sixPiVal;

    float N_inv;

};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class ExponentialFunc: public WindowFunc
{
public:
    ExponentialFunc( float _t ) {
        t_inv = 0.0;
        t = _t;
    }

    float attackWindow (int n, int attack) {
        if( n == 0 ) {
            t_inv = 1.0/(attack * t);
        }

        return exp(-abs(n - attack) * t_inv);
    }
    float releaseWindow (int n, int release) {
        if( n == 0 ) {
            t_inv = 1.0/(release * t);

            return 1.0;
        }

        return exp(-abs(n) * t_inv);
    }
private:
    float t_inv, t;
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class HannFunc : public WindowFunc
{
public:
    HannFunc() {
        two_pi = 2.0 * M_PI;
        twoPiVal = 0;
    }

    float attackWindow (int n, int attack) {
        if( n == 0 ) {
            twoPiVal = two_pi * 1.0/(attack * 2);

            return 0.0;
        }

        return 0.5 * (1.0 - cos(twoPiVal * n));
    }
    float releaseWindow (int n, int release) {
        if( n == 0 ) {
            twoPiVal = two_pi * 1.0/(release * 2);
        }

        n = n + release;

        return 0.5 * (1.0 - cos(twoPiVal * n));
    }
private:
    float two_pi;
    float twoPiVal;
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class HannPoissonFunc : public WindowFunc
{
public:
    HannPoissonFunc( float _a ) {
        a = _a;
        pi = M_PI;
        N_inv = 0.0;
    }

    float attackWindow (int n, int attack) {
        if( n == 0 ) {
            N_inv = 1.0/attack;
        }

        n = attack - n;

        return 0.5 * (1.0 + cos(pi * n * N_inv)) * exp(-a * n * N_inv);
    }
    float releaseWindow (int n, int release) {
        if( n == 0 ) {
            N_inv = 1.0/release;
        }

        return 0.5 * (1.0 + cos(pi * n * N_inv)) * exp(-a * n * N_inv);
    }
private:
    float a;
    float pi;
    float N_inv;
};

/* TODO
class KaiserFunc : public WindowFunc
{
public:
    KaiserFunc( float a ) {
    }

    float attackWindow (int n, int attack) {
        if( n == 0 )
        {
        }
        return 0;
    }
    float releaseWindow (int n, int release) {
        if( n == 0 )
        {
        }
        return 0;
    }
private:
    float a;
    float pi;
};
*/


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class ParzenFunc : public WindowFunc
{
public:
    ParzenFunc() {
        N_inv = 0.0;
    }

    float attackWindow (int n, int attack) {
        if( n == 0 ) {
            N_inv = 1.0/attack;
        }

        n = attack - n - 1;

        if( n < attack * 0.5 ) {
            return 1.0 - 6.0 * pow(n * N_inv, 2) * (1.0 - (n * N_inv));
        }
        else {
            return 2.0 * pow( 1.0 - n * N_inv, 3);
        }
    }
    float releaseWindow (int n, int release) {
        if( n == 0 ) {
            N_inv = 1.0/release;
        }

        if( n < release * 0.5 ) {
            return 1.0 - 6.0 * pow(n * N_inv, 2) * (1.0 - (n * N_inv));
        }
        else {
            return 2.0 * pow( 1.0 - n * N_inv, 3);
        }
    }
private:
    float N_inv;
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class PoissonFunc : public WindowFunc
{
public:
    PoissonFunc( float _a ) {
        a = _a;
    }

    float attackWindow (int n, int attack) {
        if( n == 0 ) {
            N_inv = 1.0/attack;
        }

        n = attack - n;

        return exp(-a * n * N_inv);
    }
    float releaseWindow (int n, int release) {
        if( n == 0 ) {
            N_inv = 1.0/release;
        }

        return exp(-a * n * N_inv);
    }
private:
    float a;
    float N_inv;
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class TukeyFunc : public WindowFunc
{
public:
    TukeyFunc( float _a ) {
        a = _a;
        pi = M_PI;
    }

    float attackWindow (int n, int attack) {
        if( n == 0 )
        {
        }
        if ( n < a * attack ) {
            return 0.5 * (1.0 + cos(pi * ((2 * n)/(a * attack * 2.0) - 1.0)));
        }
        else {
            return 1.0;
        }
    }
    float releaseWindow (int n, int release) {
        if( n == 0 )
        {
        }

        if ( n < a * release ) {
            return 1.0;
        }
        else {
            return 0.5 * (1.0 + cos(pi * ((2 * n)/(a * release * 2.0) - 2.0/a + 1.0)));
        }
    }
private:
    float a;
    float pi;
};

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class WelchFunc : public WindowFunc
{
public:
    WelchFunc() {
        N_inv = 0.0;
    }

    float attackWindow (int n, int attack) {
        if( n == 0 ) {
            N_inv = 1.0/attack;
        }

        return 1.0 - pow((n - attack) * N_inv, 2);
    }
    float releaseWindow (int n, int release) {
        if( n == 0 ) {
            N_inv = 1.0/release;
        }

        n = n + release;

        return 1.0 - pow((n - release) * N_inv, 2);
    }
private:
    float N_inv;
};

// ~~~~~~~~~~~~~
class WinFuncEnv
{
public:
    // constructor
    WinFuncEnv( t_CKFLOAT fs)
    {
        m_n = 0;

        m_keyOn = 0;
        m_keyOff = 0;

        m_attack = 0;
        m_release = 0;

        m_currentLevel = 0;
        m_keyOnLevel = 0;
        m_keyOffLevel = 0;

        m_windowValue = 0;

        // default window
        m_func = new HannFunc();
    }

    WindowFunc*  m_func;

    // keyOn
    int setKeyOn()
    {
        m_keyOnLevel = m_currentLevel;
        m_n = 0;
        m_keyOn = 1;
        m_keyOff = 0;

        return m_keyOn;
    }

    int setKeyOn( t_CKINT )
    {
        m_keyOnLevel = m_currentLevel;
        m_n = 0;
        m_keyOn = 1;
        m_keyOff = 0;

        return m_keyOn;
    }

    // keyOff
    int setKeyOff()
    {
        m_keyOffLevel = m_currentLevel;
        m_n = 0;
        m_keyOn = 0;
        m_keyOff = 1;

        return m_keyOff;
    }

    int setKeyOff( t_CKINT )
    {
        m_keyOffLevel = m_currentLevel;
        m_n = 0;
        m_keyOn = 0;
        m_keyOff = 1;

        return m_keyOff;
    }

    // set attack
    float setAttack( t_CKDUR d )
    {
        m_keyOnLevel = m_currentLevel;
        m_n = 0;

        m_attack = (float)d;

        return m_attack;
    }

    // set release
    float setRelease( t_CKDUR d )
    {
        m_keyOffLevel = m_currentLevel;
        m_n = 0;

        m_release = (float)d;

        return m_release;
    }

    // get value at
    float getWindowValue()
    {
        return m_windowValue;
    }

    void setBlackman() {
        m_func = new BlackmanFunc(0.16);
    }

    void setBlackman( t_CKFLOAT a ) {
        m_func = new BlackmanFunc(a);
    }

    void setBlackmanDerivative( t_CKFLOAT a0, t_CKFLOAT a1, t_CKFLOAT a2, t_CKFLOAT a3 ){
        m_func = new BlackmanDerivativeFunc( a0, a1, a2, a3 );
    }

    void setBlackmanHarris() {
        m_func = new BlackmanDerivativeFunc(0.35875, 0.48829, 0.14128, 0.01168);
    }

    void setBlackmanNutall() {
        m_func = new BlackmanDerivativeFunc(0.3635819, 0.4891775, 0.1365995, 0.0106411);
    }

    void setExponential() {
        m_func = new ExponentialFunc(8.69/60.0);
    }

    void setExponential( t_CKFLOAT a ) {
        m_func = new ExponentialFunc(a);
    }

    void setNutall() {
        m_func = new BlackmanDerivativeFunc(0.355768, 0.487396, 0.144232, 0.012604);
    }

    void setHann() {
        m_func = new HannFunc();
    }

    void setHannPoisson() {
        m_func = new HannPoissonFunc(0.5);
    }

    void setHannPoisson( t_CKFLOAT a ) {
        m_func = new HannPoissonFunc(a);
    }

    /* TODO
    void setKaiser() {
        m_func = new KaiserFunc(3.0);
    }
    */

    void setParzen() {
        m_func = new ParzenFunc();
    }

    void setPoisson() {
        m_func = new PoissonFunc(6.0);
    }

    void setPoisson( t_CKFLOAT a ) {
        m_func = new PoissonFunc(a);
    }

    void setTukey() {
        m_func = new TukeyFunc(0.5);
    }

    void setTukey( t_CKFLOAT a ) {
        m_func = new TukeyFunc(a);
    }

    void setWelch() {
        m_func = new WelchFunc();
    }

    // where all the action happens
    SAMPLE tick( SAMPLE in )
    {
        if (m_keyOn) {
            if (m_n < m_attack) {
                m_windowValue = m_func->attackWindow(m_n, m_attack);
                m_currentLevel = m_windowValue * (1.0 - m_keyOnLevel) + m_keyOnLevel;
                m_n++;
                return in * m_currentLevel;
            }
            else {
                return in;
            }
        }
        if (m_keyOff) {
            if (m_n < m_release) {
                m_windowValue = m_func->releaseWindow(m_n, m_release);
                m_currentLevel = m_windowValue * m_keyOffLevel;
                m_n++;
                return in * m_currentLevel;
            }
            else {
                return 0;
            }
        }
        return 0;
    }


private:
    // sample incrementor
    int m_n;

    // control vars
    int m_keyOn;
    int m_keyOff;

    // durations
    float m_attack;
    float m_release;

    // store current level
    float m_currentLevel;
    float m_keyOnLevel;
    float m_keyOffLevel;

    // store current window value
    float m_windowValue;
};


CK_DLL_QUERY( WinFuncEnv )
{
    // hmm, don't change this...
    QUERY->setname(QUERY, "WinFuncEnv");

    // begin the class definition
    // can change the second argument to extend a different ChucK class
    QUERY->begin_class(QUERY, "WinFuncEnv", "Envelope");

    // register the constructor (probably no need to change)
    QUERY->add_ctor(QUERY, winfuncenv_ctor);
    // register the destructor (probably no need to change)
    QUERY->add_dtor(QUERY, winfuncenv_dtor);

    QUERY->doc_class(QUERY, "WinFunEnv is an Attack/Release envelope built around window functions!");
    QUERY->add_ex(QUERY, "effects/WinFuncEnv.ck");

    // for UGen's only: add tick function
    QUERY->add_ugen_func(QUERY, winfuncenv_tick, NULL, 1, 1);

    // ~ envelope functions
    QUERY->add_mfun(QUERY, winfuncenv_setKeyOn, "int", "keyOn");
    QUERY->doc_func(QUERY, "Start attack phase. ");

    QUERY->add_mfun(QUERY, winfuncenv_setKeyOn, "int", "keyOn");
    QUERY->add_arg(QUERY, "int", "keyOn");
    QUERY->doc_func(QUERY, "Start attack phase. ");

    QUERY->add_mfun(QUERY, winfuncenv_setKeyOff, "int", "keyOff");
    QUERY->doc_func(QUERY, "Start release phase. ");

    QUERY->add_mfun(QUERY, winfuncenv_setKeyOff, "int", "keyOff");
    QUERY->add_arg(QUERY, "int", "keyOff");
    QUERY->doc_func(QUERY, "Start release phase. ");

    QUERY->add_mfun(QUERY, winfuncenv_setAttack, "dur", "attackTime");
    QUERY->add_arg(QUERY, "dur", "attackDuration");
    QUERY->doc_func(QUERY, "Set duration of the attack phase. ");

    QUERY->add_mfun(QUERY, winfuncenv_setRelease, "dur", "releaseTime");
    QUERY->add_arg(QUERY, "dur", "releaseDuration");
    QUERY->doc_func(QUERY, "Set duration of the release phase. ");

    QUERY->add_mfun(QUERY, winfuncenv_getWindowValue, "float", "windowValue");
    QUERY->doc_func(QUERY, "Get current window value. ");

    // ~ window functions
    QUERY->add_mfun(QUERY, winfuncenv_setBlackman, "void", "setBlackman");
    QUERY->doc_func(QUERY, "Set Blackman Window Envelope with default value (0.16). ");

    QUERY->add_mfun(QUERY, winfuncenv_setBlackmanArg, "void", "setBlackman");
    QUERY->add_arg(QUERY, "float", "a");
    QUERY->doc_func(QUERY, "Set Blackman Window Envelope with a custom value. ");

    QUERY->add_mfun(QUERY, winfuncenv_setBlackmanDerivative, "void", "setBlackmanDerivative");
    QUERY->add_arg(QUERY, "float", "a0");
    QUERY->add_arg(QUERY, "float", "a1");
    QUERY->add_arg(QUERY, "float", "a2");
    QUERY->add_arg(QUERY, "float", "a3");
    QUERY->doc_func(QUERY, "Set a custom Blackman Derivative Envelope with custom values. ");

    QUERY->add_mfun(QUERY, winfuncenv_setBlackmanHarris, "void", "setBlackmanHarris");
    QUERY->doc_func(QUERY, "Set BlackmanHarris Window Envelope. ");

    QUERY->add_mfun(QUERY, winfuncenv_setBlackmanNutall, "void", "setBlackmanNutall");
    QUERY->doc_func(QUERY, "Set BlackmanNutall Window Envelope. ");

    QUERY->add_mfun(QUERY, winfuncenv_setExponential, "void", "setExponential");
    QUERY->doc_func(QUERY, "Set Exponential Function Envelope with default value (8.69/60.0). ");

    QUERY->add_mfun(QUERY, winfuncenv_setExponentialArg, "void", "setExponential");
    QUERY->add_arg(QUERY, "float", "a");
    QUERY->doc_func(QUERY, "Set Exponential Function Envleope with a custom value. ");

    QUERY->add_mfun(QUERY, winfuncenv_setHann, "void", "setHann");
    QUERY->doc_func(QUERY, "Set Hann (Hanning) Window Envelope. ");

    QUERY->add_mfun(QUERY, winfuncenv_setHannPoisson, "void", "setHannPoisson");
    QUERY->doc_func(QUERY, "Set Hann-Poisson Window Envelope with default value (0.5). ");

    QUERY->add_mfun(QUERY, winfuncenv_setHannPoissonArg, "void", "setHannPoisson");
    QUERY->add_arg(QUERY, "float", "a");
    QUERY->doc_func(QUERY, "Set Hann-Poisson Window Envelope with a custom value. ");

    /* TODO
    QUERY->add_mfun(QUERY, winfuncenv_setHann, "void", "setKaiser");
    QUERY->doc_func(QUERY, "Sets envelope to a Kaiser Window Function. ");
    */

    QUERY->add_mfun(QUERY, winfuncenv_setNutall, "void", "setNutall");
    QUERY->doc_func(QUERY, "Set Nutall Window Envelope. ");

    QUERY->add_mfun(QUERY, winfuncenv_setParzen, "void", "setParzen");
    QUERY->doc_func(QUERY, "Set Parzen Window Envelope. ");

    QUERY->add_mfun(QUERY, winfuncenv_setPoisson, "void", "setPoisson");
    QUERY->doc_func(QUERY, "Set Poisson Window Envelope with default value (6.0). ");

    QUERY->add_mfun(QUERY, winfuncenv_setPoissonArg, "void", "setPoisson");
    QUERY->add_arg(QUERY, "float", "a");
    QUERY->doc_func(QUERY, "Set Poisson Window Envelope with a custom value. ");

    QUERY->add_mfun(QUERY, winfuncenv_setTukey, "void", "setTukey");
    QUERY->doc_func(QUERY, "Set Tukey Window Envelope with default value (0.5). ");

    QUERY->add_mfun(QUERY, winfuncenv_setTukeyArg, "void", "setTukey");
    QUERY->add_arg(QUERY, "float", "a");
    QUERY->doc_func(QUERY, "Set Tukey Window Envelope with a custom value. ");

    QUERY->add_mfun(QUERY, winfuncenv_setWelch, "void", "setWelch");
    QUERY->doc_func(QUERY, "Set Welch Window Window. ");

    // reference to the c++ class we defined above
    winfuncenv_data_offset = QUERY->add_mvar(QUERY, "int", "@wfe_data", false);

    // end the class definition
    // IMPORTANT: this MUST be called!
    QUERY->end_class(QUERY);

    // wasn't that a breeze?
    return TRUE;
}


// implementation for the constructor
CK_DLL_CTOR(winfuncenv_ctor)
{
    // get the offset where we'll store our internal c++ class pointer
    OBJ_MEMBER_INT(SELF, winfuncenv_data_offset) = 0;

    // instantiate our internal c++ class representation
    WinFuncEnv * bcdata = new WinFuncEnv(API->vm->get_srate(API, SHRED));

    // store the pointer in the ChucK object member
    OBJ_MEMBER_INT(SELF, winfuncenv_data_offset) = (t_CKINT) bcdata;
}


// implementation for the destructor
CK_DLL_DTOR(winfuncenv_dtor)
{
    // get our c++ class pointer
    WinFuncEnv * bcdata = (WinFuncEnv *) OBJ_MEMBER_INT(SELF, winfuncenv_data_offset);
    // check it
    if( bcdata )
    {
        // clean up
        delete bcdata;
        OBJ_MEMBER_INT(SELF, winfuncenv_data_offset) = 0;
        bcdata = NULL;
    }
}


// implementation for tick function
CK_DLL_TICK(winfuncenv_tick)
{
    // get our c++ class pointer
    WinFuncEnv * c = (WinFuncEnv *) OBJ_MEMBER_INT(SELF, winfuncenv_data_offset);

    // invoke our tick function; store in the magical out variable
    if(c) *out = c->tick(in);

    // yes
    return TRUE;
}

// envelope functions
CK_DLL_MFUN(winfuncenv_setKeyOn)
{
    WinFuncEnv * bcdata = (WinFuncEnv *) OBJ_MEMBER_INT(SELF, winfuncenv_data_offset);
    RETURN->v_dur = bcdata->setKeyOn(GET_NEXT_DUR(ARGS));
}

CK_DLL_MFUN(winfuncenv_setKeyOff)
{
    WinFuncEnv * bcdata = (WinFuncEnv *) OBJ_MEMBER_INT(SELF, winfuncenv_data_offset);
    RETURN->v_dur = bcdata->setKeyOff(GET_NEXT_DUR(ARGS));
}

CK_DLL_MFUN(winfuncenv_setAttack)
{
    WinFuncEnv * bcdata = (WinFuncEnv *) OBJ_MEMBER_INT(SELF, winfuncenv_data_offset);
    RETURN->v_dur = bcdata->setAttack(GET_NEXT_DUR(ARGS));
}

CK_DLL_MFUN(winfuncenv_setRelease)
{
    WinFuncEnv * bcdata = (WinFuncEnv *) OBJ_MEMBER_INT(SELF, winfuncenv_data_offset);
    RETURN->v_dur = bcdata->setRelease(GET_NEXT_DUR(ARGS));
}

CK_DLL_MFUN(winfuncenv_getWindowValue)
{
    WinFuncEnv * bcdata = (WinFuncEnv *) OBJ_MEMBER_INT(SELF, winfuncenv_data_offset);
    RETURN->v_float = bcdata->getWindowValue();
}

// window functions
CK_DLL_MFUN(winfuncenv_setBlackman)
{
    WinFuncEnv * bcdata = (WinFuncEnv *) OBJ_MEMBER_INT(SELF, winfuncenv_data_offset);
    bcdata->setBlackman();
}

CK_DLL_MFUN(winfuncenv_setBlackmanArg)
{
    WinFuncEnv * bcdata = (WinFuncEnv *) OBJ_MEMBER_INT(SELF, winfuncenv_data_offset);
    t_CKFLOAT a = GET_NEXT_FLOAT(ARGS);
    bcdata->setBlackman(a);
}

CK_DLL_MFUN(winfuncenv_setBlackmanDerivative)
{
    WinFuncEnv * bcdata = (WinFuncEnv *) OBJ_MEMBER_INT(SELF, winfuncenv_data_offset);
    t_CKFLOAT a0 = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT a1 = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT a2 = GET_NEXT_FLOAT(ARGS);
    t_CKFLOAT a3 = GET_NEXT_FLOAT(ARGS);
    bcdata->setBlackmanDerivative(a0, a1, a2, a3);
}

CK_DLL_MFUN(winfuncenv_setBlackmanHarris)
{
    WinFuncEnv * bcdata = (WinFuncEnv *) OBJ_MEMBER_INT(SELF, winfuncenv_data_offset);
    bcdata->setBlackmanHarris();
}

CK_DLL_MFUN(winfuncenv_setBlackmanNutall)
{
    WinFuncEnv * bcdata = (WinFuncEnv *) OBJ_MEMBER_INT(SELF, winfuncenv_data_offset);
    bcdata->setBlackmanNutall();
}

CK_DLL_MFUN(winfuncenv_setExponential)
{
    WinFuncEnv * bcdata = (WinFuncEnv *) OBJ_MEMBER_INT(SELF, winfuncenv_data_offset);
    bcdata->setExponential();
}

CK_DLL_MFUN(winfuncenv_setExponentialArg)
{
    WinFuncEnv * bcdata = (WinFuncEnv *) OBJ_MEMBER_INT(SELF, winfuncenv_data_offset);
    t_CKFLOAT a = GET_NEXT_FLOAT(ARGS);
    bcdata->setExponential(a);
}

CK_DLL_MFUN(winfuncenv_setHann)
{
    WinFuncEnv * bcdata = (WinFuncEnv *) OBJ_MEMBER_INT(SELF, winfuncenv_data_offset);
    bcdata->setHann();
}

CK_DLL_MFUN(winfuncenv_setHannPoisson)
{
    WinFuncEnv * bcdata = (WinFuncEnv *) OBJ_MEMBER_INT(SELF, winfuncenv_data_offset);
    bcdata->setHannPoisson();
}

CK_DLL_MFUN(winfuncenv_setHannPoissonArg)
{
    WinFuncEnv * bcdata = (WinFuncEnv *) OBJ_MEMBER_INT(SELF, winfuncenv_data_offset);
    t_CKFLOAT a = GET_NEXT_FLOAT(ARGS);
    bcdata->setHannPoisson(a);
}

/* TODO
CK_DLL_MFUN(winfuncenv_setKaiser)
{
    WinFuncEnv * bcdata = (WinFuncEnv *) OBJ_MEMBER_INT(SELF, winfuncenv_data_offset);
    bcdata->setKaiser();
}
*/

CK_DLL_MFUN(winfuncenv_setNutall)
{
    WinFuncEnv * bcdata = (WinFuncEnv *) OBJ_MEMBER_INT(SELF, winfuncenv_data_offset);
    bcdata->setNutall();
}

CK_DLL_MFUN(winfuncenv_setParzen)
{
    WinFuncEnv * bcdata = (WinFuncEnv *) OBJ_MEMBER_INT(SELF, winfuncenv_data_offset);
    bcdata->setParzen();
}

CK_DLL_MFUN(winfuncenv_setPoisson)
{
    WinFuncEnv * bcdata = (WinFuncEnv *) OBJ_MEMBER_INT(SELF, winfuncenv_data_offset);
    bcdata->setPoisson();
}

CK_DLL_MFUN(winfuncenv_setPoissonArg)
{
    WinFuncEnv * bcdata = (WinFuncEnv *) OBJ_MEMBER_INT(SELF, winfuncenv_data_offset);
    t_CKFLOAT a = GET_NEXT_FLOAT(ARGS);
    bcdata->setPoisson(a);
}

CK_DLL_MFUN(winfuncenv_setTukey)
{
    WinFuncEnv * bcdata = (WinFuncEnv *) OBJ_MEMBER_INT(SELF, winfuncenv_data_offset);
    bcdata->setTukey();
}

CK_DLL_MFUN(winfuncenv_setTukeyArg)
{
    WinFuncEnv * bcdata = (WinFuncEnv *) OBJ_MEMBER_INT(SELF, winfuncenv_data_offset);
    t_CKFLOAT a = GET_NEXT_FLOAT(ARGS);
    bcdata->setTukey(a);
}


CK_DLL_MFUN(winfuncenv_setWelch)
{
    WinFuncEnv * bcdata = (WinFuncEnv *) OBJ_MEMBER_INT(SELF, winfuncenv_data_offset);
    bcdata->setWelch();
}
