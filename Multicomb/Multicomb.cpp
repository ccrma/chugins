//-----------------------------------------------------------------------------
// Entaro ChucK Developer!
// This is a Chugin boilerplate, generated by chuginate!
//-----------------------------------------------------------------------------

// this should align with the correct versions of these ChucK files
#include "chugin.h"
#include "Ocomb.h"

#define NCOMBS      5
#define DEF_MINFREQ 220
#define DEF_MAXFREQ 880
#define DEF_REVTIME 1

// general includes
#include <stdio.h>
#include <limits.h>

#ifdef __PLATFORM_WINDOWS__
static long random() { return rand(); }
static void srandom( unsigned s ) { srand( s ); }
#define MULTICOMB_RANDOM_MAX RAND_MAX
#else
// on other platforms random()'s max is (2^31)-1
#define MULTICOMB_RANDOM_MAX 0x7fffffff
#endif // _MSC_VER

CK_DLL_CTOR(multicomb_ctor);
CK_DLL_DTOR(multicomb_dtor);
CK_DLL_TICKF(multicomb_tickf);

CK_DLL_MFUN(multicomb_setNum);
CK_DLL_MFUN(multicomb_setMinfreq);
CK_DLL_MFUN(multicomb_setMaxfreq);
CK_DLL_MFUN(multicomb_setRange);
CK_DLL_MFUN(multicomb_setRevtime);

CK_DLL_MFUN(multicomb_getNum);
CK_DLL_MFUN(multicomb_getMinfreq);
CK_DLL_MFUN(multicomb_getMaxfreq);
CK_DLL_MFUN(multicomb_getRevtime);

t_CKINT multicomb_data_offset = 0;

// class definition for internal Chugin data
class Multicomb
{
public:
  Multicomb( t_CKFLOAT fs)
  {
    _srate = fs;
	_num = NCOMBS;
    _minfreq = DEF_MINFREQ;
    _maxfreq = DEF_MAXFREQ;
    _revtime = DEF_REVTIME;
    _spread = new float[_num];
    _delsamps = new int[_num];
    _comb = new Ocomb*[_num];
    for (int i=0; i< _num; i++)
      {
		_spread[i] = (float) i / (float) (_num-1);
		float cfreq = rand2f(_minfreq,_maxfreq);
		float loopt = 1.0 / cfreq;
		_delsamps[i] = (int) (loopt * _srate + 0.5);
		_comb[i] = new Ocomb(_srate, loopt, _revtime);
		if (_comb[i]->frequency() == 0.0)
		  printf("Multicomb error: comb delay allocation delay failed.\n");
      }
  }

  ~Multicomb()
  {
	delete [] _spread;
	delete [] _delsamps;
	for (int i=0; i<_num; i++)
	  {
		delete _comb[i];
	  }
	delete [] _comb;
  }

  void tickf( SAMPLE* in, SAMPLE* out, int nframes)
  {
    memset (out, 0, sizeof(SAMPLE)*nframes);
    out[0] = out[1] = 0.0;
    for (int i=0; i<_num; i++)
      {
	float sig = _comb[i]->next(in[0],_delsamps[i]);
	out[0] += sig * _spread[i];
	out[1] += sig * (1.0 - _spread[i]);
      }
  }
  
  t_CKDUR setRevtime ( t_CKDUR p)
  {
    _revtime = (p/_srate);
    for (int i=0; i<_num; i++)
		_comb[i]->setReverbTime(_revtime);
    return p;
  }

  void setRange ( t_CKFLOAT lo, t_CKFLOAT hi )
  {
	_minfreq = lo;
	_maxfreq = hi;
    for (int i=0; i< _num; i++)
      {
		float cfreq = rand2f(_minfreq,_maxfreq);
		float loopt = 1.0 / cfreq;
		_delsamps[i] = (int) (loopt * _srate + 0.5);
		_comb[i] = new Ocomb(_srate, loopt, _revtime);
		if (_comb[i]->frequency() == 0.0)
		  printf("Multicomb error: comb delay allocation delay failed.\n");
      }
  }

  float setMinfreq ( t_CKFLOAT p )
  {
	_minfreq = p;
    for (int i=0; i< _num; i++)
      {
		float cfreq = rand2f(_minfreq,_maxfreq);
		float loopt = 1.0 / cfreq;
		_delsamps[i] = (int) (loopt * _srate + 0.5);
		_comb[i] = new Ocomb(_srate, loopt, _revtime);
		if (_comb[i]->frequency() == 0.0)
		  printf("Multicomb error: comb delay allocation delay failed.\n");
      }
	return p;
  }

  float setMaxfreq ( t_CKFLOAT p )
  {
	_maxfreq = p;
    for (int i=0; i< _num; i++)
      {
		float cfreq = rand2f(_minfreq,_maxfreq);
		float loopt = 1.0 / cfreq;
		_delsamps[i] = (int) (loopt * _srate + 0.5);
		_comb[i] = new Ocomb(_srate, loopt, _revtime);
		if (_comb[i]->frequency() == 0.0)
		  printf("Multicomb error: comb delay allocation delay failed.\n");
      }
	return p;
  }

  int setNum ( t_CKINT p )
  {
	delete [] _spread;
	delete [] _delsamps;
	for (int i=0; i<_num; i++)
	  {
		delete _comb[i];
	  }
	delete [] _comb;
	if (p>0) _num = p;		
    _spread = new float[_num];
    _delsamps = new int[_num];
    _comb = new Ocomb*[_num];
    for (int i=0; i< _num; i++)
      {
		_spread[i] = (float) i / (float) (_num-1);
		float cfreq = rand2f(_minfreq,_maxfreq);
		float loopt = 1.0 / cfreq;
		_delsamps[i] = (int) (loopt * _srate + 0.5);
		_comb[i] = new Ocomb(_srate, loopt, _revtime);
		if (_comb[i]->frequency() == 0.0)
		  printf("Multicomb error: comb delay allocation delay failed.\n");
      }
    return _num;
  }	
  
  int getNum() { return _num; }
  float getMinfreq() { return _minfreq; }
  float getMaxfreq() { return _maxfreq; }
  t_CKDUR getRevtime() { return _revtime * _srate; }
  
private:

  float rand2f (float min, float max)
  {
    return min + (max-min)*(::random()/(t_CKFLOAT)MULTICOMB_RANDOM_MAX);
  }
  unsigned int _num;
  float _srate;
  Ocomb **_comb;
  int *_delsamps;
  float *_spread;
  float _minfreq, _maxfreq;
  float _revtime;
  float m_param;
};

// query function: chuck calls this when loading the Chugin
// NOTE: developer will need to modify this function to
// add additional functions to this Chugin
CK_DLL_QUERY( Multicomb )
{
  // hmm, don't change this...
  QUERY->setname(QUERY, "Multicomb");
  
  // begin the class definition
  // can change the second argument to extend a different ChucK class
  QUERY->begin_class(QUERY, "Multicomb", "UGen");
  
  // register the constructor (probably no need to change)
  QUERY->add_ctor(QUERY, multicomb_ctor);
  // register the destructor (probably no need to change)
  QUERY->add_dtor(QUERY, multicomb_dtor);

  QUERY->doc_class(QUERY, "Multiple simultaneous comb filters randomly "
                   "chosen within a specified frequency range "
                   "and spread across the stereo field.");
  QUERY->add_ex(QUERY, "filter/Multicomb.ck");
  
  // for UGen's only: add tickf function
  QUERY->add_ugen_funcf(QUERY, multicomb_tickf, NULL, 2, 2);
  
  // NOTE: if this is to be a UGen with more than 1 channel, 
  // e.g., a multichannel UGen -- will need to use add_ugen_funcf()
  // and declare a tickf function using CK_DLL_TICKF
  
  QUERY->add_mfun(QUERY, multicomb_setNum, "int", "num");
  QUERY->add_arg(QUERY, "int", "num");
  QUERY->doc_func(QUERY, "Set number of comb filters. Default 5.");

  QUERY->add_mfun(QUERY, multicomb_setMinfreq, "float", "minfreq");
  QUERY->add_arg(QUERY, "float", "minfreq");
  QUERY->doc_func(QUERY, "Set low frequency. Default 220.");

  QUERY->add_mfun(QUERY, multicomb_setMaxfreq, "float", "maxfreq");
  QUERY->add_arg(QUERY, "float", "maxfreq");
  QUERY->doc_func(QUERY, "Set max frequency. Default 880.");

  QUERY->add_mfun(QUERY, multicomb_setRange, "void", "set");
  QUERY->add_arg(QUERY, "float", "minfreq");
  QUERY->add_arg(QUERY, "float", "maxfreq");
  QUERY->doc_func(QUERY, "Set both low and high frequencies.");  

  QUERY->add_mfun(QUERY, multicomb_setRevtime, "dur", "revtime");
  QUERY->add_arg(QUERY, "dur", "revtime");
  QUERY->doc_func(QUERY, "Set total ring time. Default 1::second.");
  
  // example of adding getter method
  QUERY->add_mfun(QUERY, multicomb_getNum, "int", "num");
  QUERY->doc_func(QUERY, "Get number of comb filters. Default 5.");
  QUERY->add_mfun(QUERY, multicomb_getMinfreq, "float", "minfreq");
  QUERY->doc_func(QUERY, "Get low frequency. Default 220.");
  QUERY->add_mfun(QUERY, multicomb_getMaxfreq, "float", "maxfreq");
  QUERY->doc_func(QUERY, "Get max frequency. Default 880.");
  QUERY->add_mfun(QUERY, multicomb_getRevtime, "dur", "revtime");
  QUERY->doc_func(QUERY, "Get total ring time. Default 1::second.");
  
  // this reserves a variable in the ChucK internal class to store 
  // referene to the c++ class we defined above
  multicomb_data_offset = QUERY->add_mvar(QUERY, "int", "@m_data", false);
  
  // end the class definition
  // IMPORTANT: this MUST be called!
  QUERY->end_class(QUERY);
  
  // wasn't that a breeze?
  return TRUE;
}


// implementation for the constructor
CK_DLL_CTOR(multicomb_ctor)
{
  // get the offset where we'll store our internal c++ class pointer
  OBJ_MEMBER_INT(SELF, multicomb_data_offset) = 0;
  
  // instantiate our internal c++ class representation
  Multicomb * bcdata = new Multicomb(API->vm->srate(VM));
  
  // store the pointer in the ChucK object member
  OBJ_MEMBER_INT(SELF, multicomb_data_offset) = (t_CKINT) bcdata;
}


// implementation for the destructor
CK_DLL_DTOR(multicomb_dtor)
{
  // get our c++ class pointer
  Multicomb * bcdata = (Multicomb *) OBJ_MEMBER_INT(SELF, multicomb_data_offset);
  // check it
  if( bcdata )
    {
	  // clean up
	  delete bcdata;
	  OBJ_MEMBER_INT(SELF, multicomb_data_offset) = 0;
	  bcdata = NULL;
    }
}

CK_DLL_TICKF(multicomb_tickf)
{
  Multicomb * c = (Multicomb *) OBJ_MEMBER_INT(SELF, multicomb_data_offset);
  if(c) c->tickf(in,out, nframes);
  return TRUE;
}

CK_DLL_MFUN(multicomb_setNum)
{
  Multicomb * bcdata = (Multicomb *) OBJ_MEMBER_INT(SELF, multicomb_data_offset);
  RETURN->v_int = bcdata->setNum(GET_NEXT_INT(ARGS));
}

CK_DLL_MFUN(multicomb_setMinfreq)
{
  Multicomb * bcdata = (Multicomb *) OBJ_MEMBER_INT(SELF, multicomb_data_offset);
  RETURN->v_float = bcdata->setMinfreq(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(multicomb_setMaxfreq)
{
  Multicomb * bcdata = (Multicomb *) OBJ_MEMBER_INT(SELF, multicomb_data_offset);
  RETURN->v_float = bcdata->setMaxfreq(GET_NEXT_FLOAT(ARGS));
}

CK_DLL_MFUN(multicomb_setRange)
{
  Multicomb * bcdata = (Multicomb *) OBJ_MEMBER_INT(SELF, multicomb_data_offset);
  float high = GET_NEXT_FLOAT(ARGS);
  float low = GET_NEXT_FLOAT(ARGS);
  bcdata->setRange(low,high);
}

CK_DLL_MFUN(multicomb_setRevtime)
{
  Multicomb * bcdata = (Multicomb *) OBJ_MEMBER_INT(SELF, multicomb_data_offset);
  RETURN->v_dur = bcdata->setRevtime(GET_NEXT_DUR(ARGS));
}

// Get methods
CK_DLL_MFUN(multicomb_getNum)
{
  Multicomb * bcdata = (Multicomb *) OBJ_MEMBER_INT(SELF, multicomb_data_offset);
  RETURN->v_int = bcdata->getNum();
}

CK_DLL_MFUN(multicomb_getMinfreq)
{
  Multicomb * bcdata = (Multicomb *) OBJ_MEMBER_INT(SELF, multicomb_data_offset);
  RETURN->v_float = bcdata->getMinfreq();
}

CK_DLL_MFUN(multicomb_getMaxfreq)
{
  Multicomb * bcdata = (Multicomb *) OBJ_MEMBER_INT(SELF, multicomb_data_offset);
  RETURN->v_float = bcdata->getMaxfreq();
}

CK_DLL_MFUN(multicomb_getRevtime)
{
  Multicomb * bcdata = (Multicomb *) OBJ_MEMBER_INT(SELF, multicomb_data_offset);
  RETURN->v_dur = bcdata->getRevtime();
}
