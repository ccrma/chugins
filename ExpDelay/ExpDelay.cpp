//-----------------------------------------------------------------------------
// Entaro ChucK Developer!
// This is a Chugin boilerplate, generated by chuginate!
//-----------------------------------------------------------------------------

// this should align with the correct versions of these ChucK files
#include "chugin.h"

// general includes
#include <stdio.h>
#include <limits.h>
#include <math.h> // for pow()

#define DEF_CURVE      1
#define DEF_NUMPTS    20
#define DEF_BUFLEN 88200
#define MIN_BUFLEN   100
#define CLIP(a, lo, hi) ( (a)>(lo)?( (a)<(hi)?(a):(hi) ):(lo) )

// declaration of chugin constructor
CK_DLL_CTOR(expdelay_ctor);
// declaration of chugin desctructor
CK_DLL_DTOR(expdelay_dtor);

CK_DLL_MFUN(expdelay_setMix);
CK_DLL_MFUN(expdelay_setDurCurve);
CK_DLL_MFUN(expdelay_setAmpCurve);
CK_DLL_MFUN(expdelay_setDelay);
CK_DLL_MFUN(expdelay_setMaxDelay);
CK_DLL_MFUN(expdelay_setReps);

CK_DLL_MFUN(expdelay_getMix);
CK_DLL_MFUN(expdelay_getDurCurve);
CK_DLL_MFUN(expdelay_getAmpCurve);
CK_DLL_MFUN(expdelay_getDelay);
CK_DLL_MFUN(expdelay_getMaxDelay);
CK_DLL_MFUN(expdelay_getReps);

// for Chugins extending UGen, this is mono synthesis function for 1 sample
CK_DLL_TICK(expdelay_tick);

// this is a special offset reserved for Chugin internal data
t_CKINT expdelay_data_offset = 0;


// class definition for internal Chugin data
// (note: this isn't strictly necessary, but serves as example
// of one recommended approach)
class ExpDelay
{
public:
  // constructor
  ExpDelay( t_CKFLOAT fs)
  {
    _durcurve = DEF_CURVE;
    _ampcurve = DEF_CURVE;
    _maxbuflen = DEF_BUFLEN;
    _buflen = _maxbuflen;
    _bufIndex = 0;
    _numpts = DEF_NUMPTS;
    _delbuf = new SAMPLE[_maxbuflen];
    for (int i=0; i<_maxbuflen; i++)
      _delbuf[i] = 0.0f;
    double amptotal = 0;
    for (int i=0; i<_numpts; i++)
      {
	amptotal += experp (i, 0, _numpts, _ampcurve, 1, 0);
      }
    _gainscale = (float) (1.0/amptotal);
    _mix = 1.0f;
  }

  ~ExpDelay()
  {
    delete [] _delbuf;
  }
  
  // for Chugins extending UGen
  SAMPLE tick( SAMPLE in )
  {
    // default: this passes whatever input is patched into Chugin
    for (int i=0; i<_numpts; i++)
      {
	float amp = experp (i, 0, _numpts, _ampcurve, 1, 0);
	unsigned int bufpt = (unsigned int)experp (i, 0, _numpts, _durcurve, 0, _buflen);
	unsigned int adjustedBufpt = (bufpt + _bufIndex) % _buflen;
	_delbuf[adjustedBufpt] += in * amp * _gainscale;
      }
    SAMPLE out = _delbuf[_bufIndex];
    _delbuf[_bufIndex] = 0.0f;
    _bufIndex++;
    if (_bufIndex >= _buflen) _bufIndex = 0;
    return (out*_mix) + (in*(1-_mix));
  }
  
  float setMix ( t_CKFLOAT p )
  {
    _mix = CLIP(p,0.0f,1.0f);
    return _mix;
  }

  int setReps( t_CKINT p )
  {
    if (p < 1) p = 1;
    _numpts = p;
    double amptotal = 0;
    for (int i=0; i<_numpts; i++)
      {
	amptotal += experp (i, 0, _numpts, _ampcurve, 1, 0);
      }
    _gainscale = (float) (1.0/amptotal);
    return _numpts;
  }

  float setDurCurve( t_CKFLOAT p )
  {
    if (p <= 0) p=0.0001;
    _durcurve = p;
    return _durcurve;
  }

  float setAmpCurve( t_CKFLOAT p )
  {
    if (p <= 0) p=0.0001;
    _ampcurve = p;
    double amptotal = 0;
    for (int i=0; i<_numpts; i++)
      {
	amptotal += experp (i, 0, _numpts, _ampcurve, 1, 0);
      }
    _gainscale = (float) (1.0/amptotal);
    return _ampcurve;
  }

  t_CKDUR setDelay ( t_CKDUR p )
  {
    unsigned int temp = (unsigned int)p;
    if (temp < MIN_BUFLEN)
      {
	printf("ExpDelay error: delay must be greater than %d samples.\n",MIN_BUFLEN);
	return p;
      }
    if (temp > _maxbuflen)
      {
	printf("ExpDelay error: cannot set delay greater than max delay.\n");
	return p;
      }
    _buflen = temp;
    for (int i=_buflen; i<_maxbuflen; i++)
      _delbuf[i] = 0.0f;
    return _buflen;
  }

  t_CKDUR setMaxDelay ( t_CKDUR p )
  {
    _maxbuflen = (unsigned int)p;
    if (_maxbuflen < MIN_BUFLEN) _maxbuflen = MIN_BUFLEN;
    delete [] _delbuf;
    _delbuf = new SAMPLE[_maxbuflen];
    for (int i=0; i<_maxbuflen; i++)
      _delbuf[i] = 0.0f;
    return p;
  }
  
  float getMix() { return _mix; }
  float getReps() { return _numpts; }
  float getDurCurve() { return _durcurve; }
  float getAmpCurve() { return _ampcurve; }
  t_CKDUR getDelay() { return (t_CKDUR)_buflen; }
  t_CKDUR getMaxDelay() { return (t_CKDUR)_maxbuflen; }
  
private:
  // exponential interpolation
  double experp (double inval, double inlo, double inhi, double curve, double outlo, double outhi)
  {
    double lerp = (inval - inlo) / (inhi - inlo);
    double expval = pow (lerp, curve);
    return expval * (outhi - outlo) + outlo;
  }
  // instance data
  SAMPLE * _delbuf;
  unsigned int _buflen, _maxbuflen;
  unsigned int _bufIndex;
  float _durcurve, _ampcurve;
  unsigned int _numpts;
  float _gainscale;
  float _mix;
};


// query function: chuck calls this when loading the Chugin
// NOTE: developer will need to modify this function to
// add additional functions to this Chugin
CK_DLL_QUERY( ExpDelay )
{
  // hmm, don't change this...
  QUERY->setname(QUERY, "ExpDelay");
  
  // begin the class definition
  // can change the second argument to extend a different ChucK class
  QUERY->begin_class(QUERY, "ExpDelay", "UGen");
  
  // register the constructor (probably no need to change)
  QUERY->add_ctor(QUERY, expdelay_ctor);
  // register the destructor (probably no need to change)
  QUERY->add_dtor(QUERY, expdelay_dtor);

  QUERY->doc_class(QUERY, "Feedback delay at exponentially changing delay times.");
  QUERY->add_ex(QUERY, "effects/ExpDelay.ck");
  
  // for UGen's only: add tick function
  QUERY->add_ugen_func(QUERY, expdelay_tick, NULL, 1, 1);
  
  // NOTE: if this is to be a UGen with more than 1 channel, 
  // e.g., a multichannel UGen -- will need to use add_ugen_funcf()
  // and declare a tickf function using CK_DLL_TICKF

  QUERY->add_mfun(QUERY, expdelay_setMix, "float", "mix");
  QUERY->add_arg(QUERY, "float", "mix");
  QUERY->doc_func(QUERY, "Set dry/wet mix [0-1].");

  QUERY->add_mfun(QUERY, expdelay_setReps, "int", "reps");
  QUERY->add_arg(QUERY, "int", "reps");
  QUERY->doc_func(QUERY, "Set number of repetitions.");
  
  QUERY->add_mfun(QUERY, expdelay_setDurCurve, "float", "durcurve");
  QUERY->add_arg(QUERY, "float", "durcurve");
  QUERY->doc_func(QUERY, "Set steepness of delay curve [0.0001-inf]:\n"
                  "       1 = steady\n"
                  "      <1 = starts fast and slows down\n"
                  "      >1 = starts slow and speeds up"
                  );

  QUERY->add_mfun(QUERY, expdelay_setAmpCurve, "float", "ampcurve");
  QUERY->add_arg(QUERY, "float", "ampcurve");
  QUERY->doc_func(QUERY, "Set steepness of amplitude decay [0.0001-inf].");
  
  QUERY->add_mfun(QUERY, expdelay_setDelay, "dur", "delay");
  QUERY->add_arg(QUERY, "dur", "delay");
  QUERY->doc_func(QUERY, "Set duration of delay.");

  QUERY->add_mfun(QUERY, expdelay_setMaxDelay, "dur", "max");
  QUERY->add_arg(QUERY, "dur", "max");
  QUERY->doc_func(QUERY, "Set maximum possible delay duration.");

  QUERY->add_mfun(QUERY, expdelay_getMix, "float", "mix");
  QUERY->doc_func(QUERY, "Get dry/wet mix [0-1].");
  QUERY->add_mfun(QUERY, expdelay_getReps, "int", "reps");
  QUERY->doc_func(QUERY, "Get number of repetitions.");
  QUERY->add_mfun(QUERY, expdelay_getDurCurve, "float", "durcurve");
  QUERY->doc_func(QUERY, "Get steepness of delay curve [0.0001-inf].");
  QUERY->add_mfun(QUERY, expdelay_getAmpCurve, "float", "ampcurve");
  QUERY->doc_func(QUERY, "Get steepness of amplitude decay [0.0001-inf].");
  QUERY->add_mfun(QUERY, expdelay_getDelay, "dur", "delay");
  QUERY->doc_func(QUERY, "Get duration of delay.");
  QUERY->add_mfun(QUERY, expdelay_getMaxDelay, "dur", "max");
  QUERY->doc_func(QUERY, "Get maximum possible delay duration.");

  // this reserves a variable in the ChucK internal class to store 
  // referene to the c++ class we defined above
  expdelay_data_offset = QUERY->add_mvar(QUERY, "int", "@ed_data", false);
  
  // end the class definition
  // IMPORTANT: this MUST be called!
  QUERY->end_class(QUERY);
  
  // wasn't that a breeze?
  return TRUE;
}


// implementation for the constructor
CK_DLL_CTOR(expdelay_ctor)
{
  // get the offset where we'll store our internal c++ class pointer
  OBJ_MEMBER_INT(SELF, expdelay_data_offset) = 0;
  
  // instantiate our internal c++ class representation
  ExpDelay * bcdata = new ExpDelay(API->vm->srate(VM));
  
  // store the pointer in the ChucK object member
  OBJ_MEMBER_INT(SELF, expdelay_data_offset) = (t_CKINT) bcdata;
}


// implementation for the destructor
CK_DLL_DTOR(expdelay_dtor)
{
  // get our c++ class pointer
  ExpDelay * bcdata = (ExpDelay *) OBJ_MEMBER_INT(SELF, expdelay_data_offset);
  // check it
  if( bcdata )
    {
      // clean up
      delete bcdata;
      OBJ_MEMBER_INT(SELF, expdelay_data_offset) = 0;
      bcdata = NULL;
    }
}


// implementation for tick function
CK_DLL_TICK(expdelay_tick)
{
  // get our c++ class pointer
  ExpDelay * c = (ExpDelay *) OBJ_MEMBER_INT(SELF, expdelay_data_offset);
  
  // invoke our tick function; store in the magical out variable
  if(c) *out = c->tick(in);
  
  // yes
  return TRUE;
}

// set methods
CK_DLL_MFUN(expdelay_setMix)
{
  ExpDelay * bcdata = (ExpDelay *) OBJ_MEMBER_INT(SELF, expdelay_data_offset);
  RETURN->v_float = bcdata->setMix(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(expdelay_setReps)
{
  ExpDelay * bcdata = (ExpDelay *) OBJ_MEMBER_INT(SELF, expdelay_data_offset);
  RETURN->v_int = bcdata->setReps(GET_NEXT_INT(ARGS));
}

CK_DLL_MFUN(expdelay_setDurCurve)
{
  ExpDelay * bcdata = (ExpDelay *) OBJ_MEMBER_INT(SELF, expdelay_data_offset);
  RETURN->v_float = bcdata->setDurCurve(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(expdelay_setAmpCurve)
{
  ExpDelay * bcdata = (ExpDelay *) OBJ_MEMBER_INT(SELF, expdelay_data_offset);
  RETURN->v_float = bcdata->setAmpCurve(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(expdelay_setDelay)
{
  ExpDelay * bcdata = (ExpDelay *) OBJ_MEMBER_INT(SELF, expdelay_data_offset);
  RETURN->v_dur = bcdata->setDelay(GET_NEXT_DUR(ARGS));
}

CK_DLL_MFUN(expdelay_setMaxDelay)
{
  ExpDelay * bcdata = (ExpDelay *) OBJ_MEMBER_INT(SELF, expdelay_data_offset);
  RETURN->v_dur = bcdata->setMaxDelay(GET_NEXT_DUR(ARGS));
}

// get methods
CK_DLL_MFUN(expdelay_getMix)
{
  ExpDelay * bcdata = (ExpDelay *) OBJ_MEMBER_INT(SELF, expdelay_data_offset);
  RETURN->v_float = bcdata->getMix();
}

CK_DLL_MFUN(expdelay_getReps)
{
  ExpDelay * bcdata = (ExpDelay *) OBJ_MEMBER_INT(SELF, expdelay_data_offset);
  RETURN->v_int = bcdata->getReps();
}

CK_DLL_MFUN(expdelay_getDurCurve)
{
  ExpDelay * bcdata = (ExpDelay *) OBJ_MEMBER_INT(SELF, expdelay_data_offset);
  RETURN->v_float = bcdata->getDurCurve();
}

CK_DLL_MFUN(expdelay_getAmpCurve)
{
  ExpDelay * bcdata = (ExpDelay *) OBJ_MEMBER_INT(SELF, expdelay_data_offset);
  RETURN->v_float = bcdata->getAmpCurve();
}

CK_DLL_MFUN(expdelay_getDelay)
{
  ExpDelay * bcdata = (ExpDelay *) OBJ_MEMBER_INT(SELF, expdelay_data_offset);
  RETURN->v_dur = bcdata->getDelay();
}

CK_DLL_MFUN(expdelay_getMaxDelay)
{
  ExpDelay * bcdata = (ExpDelay *) OBJ_MEMBER_INT(SELF, expdelay_data_offset);
  RETURN->v_dur = bcdata->getMaxDelay();
}
