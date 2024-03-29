//-----------------------------------------------------------------------------
// Entaro ChucK Developer!
// This is a Chugin boilerplate, generated by chuginate!
//-----------------------------------------------------------------------------

// this should align with the correct versions of these ChucK files
#include "chugin.h"

#include "gverbdsp.h"
#include "gverbdefs.h"

// general includes
#include <stdio.h>
#include <limits.h>
//#include <stdlib.h>

// declaration of chugin constructor
CK_DLL_CTOR(gverb_ctor);
// declaration of chugin destructor
CK_DLL_DTOR(gverb_dtor);

// example of getter/setter
CK_DLL_MFUN(gverb_setRoomsize);
CK_DLL_MFUN(gverb_getRoomsize);
CK_DLL_MFUN(gverb_setRevTime);
CK_DLL_MFUN(gverb_getRevTime);
CK_DLL_MFUN(gverb_setDamping);
CK_DLL_MFUN(gverb_getDamping);
CK_DLL_MFUN(gverb_setBandwidth);
CK_DLL_MFUN(gverb_getBandwidth);
CK_DLL_MFUN(gverb_setDryLevel);
CK_DLL_MFUN(gverb_getDryLevel);
CK_DLL_MFUN(gverb_setEarlyLevel);
CK_DLL_MFUN(gverb_getEarlyLevel);
CK_DLL_MFUN(gverb_setTailLevel);
CK_DLL_MFUN(gverb_getTailLevel);

// for Chugins extending UGen, this is mono synthesis function for 1 sample
CK_DLL_TICKF(gverb_tick);

// this is a special offset reserved for Chugin internal data
t_CKINT gverb_data_offset = 0;


// class definition for internal Chugin data
// (note: this isn't strictly necessary, but serves as example
// of one recommended approach)
class GVerb
{
public:
    // constructor
    GVerb( t_CKFLOAT fs)
    {
      
      const float maxroomsize = 300.0f;
      float roomsize = 30.0f;
      float revtime = 5.0f;
      float damping = 0.8f;
      float spread = 15.0f;
      float inputbandwidth = 0.5f;
      float drylevel = 0.6f; //-1.9832f;
      float earlylevel = 0.4f; //-1.9832f;
      float taillevel = 0.5f;
      
      float ga,gb,gt;
      unsigned int i;
      int n;
      float r;
      float diffscale;
      int a,b,c,cc,d,dd,e;
      float spread1,spread2;
      
      p = &realp;
      memset((void *)p, 0, sizeof (ty_gverb));
      p->rate = fs;
      
      p->fdndamping = damping;
      p->maxroomsize = maxroomsize;
      p->roomsize = CLIP(roomsize, 0.1f, maxroomsize);
      p->revtime = revtime;
      p->drylevel = drylevel;
      p->earlylevel = earlylevel;
      p->taillevel = taillevel;
      
      p->maxdelay = p->rate*p->maxroomsize/340.0;
      p->largestdelay = p->rate*p->roomsize/340.0;
      
      /* Input damper */
      
      p->inputbandwidth = inputbandwidth;
      p->inputdamper = damper_make(1.0 - p->inputbandwidth);
      
   	/* FDN section */

	p->fdndels = (ty_fixeddelay **)malloc(FDNORDER*sizeof(ty_fixeddelay *));
	for(i = 0; i < FDNORDER; i++)
	{
		p->fdndels[i] = fixeddelay_make((int)p->maxdelay+1000);
	}
	p->fdngains = (float *)malloc(FDNORDER*sizeof(float));
	p->fdnlens = (int *)malloc(FDNORDER*sizeof(int));

	p->fdndamps = (ty_damper **)malloc(FDNORDER*sizeof(ty_damper *));

	for(i = 0; i < FDNORDER; i++)
	{
		p->fdndamps[i] = damper_make(p->fdndamping);
	}

	ga = 60.0;
	gt = p->revtime;
	ga = pow(10.0,-ga/20.0);
	n = (int)(p->rate*gt);
	p->alpha = pow((double)ga,(double)1.0/(double)n);

	gb = 0.0;
	for(i = 0; i < FDNORDER; i++)
	{
		if (i == 0) gb = 1.000000*p->largestdelay;
		if (i == 1) gb = 0.816490*p->largestdelay;
		if (i == 2) gb = 0.707100*p->largestdelay;
		if (i == 3) gb = 0.632450*p->largestdelay;

#if 0
		p->fdnlens[i] = nearest_prime((int)gb, 0.5);
#else
		p->fdnlens[i] = (int)gb;
#endif
		// p->fdngains[i] = -pow(p->alpha,(double)p->fdnlens[i]);
		p->fdngains[i] = -powf((float)p->alpha,p->fdnlens[i]);
	}

	p->d = (float *)malloc(FDNORDER*sizeof(float));
	p->u = (float *)malloc(FDNORDER*sizeof(float));
	p->f = (float *)malloc(FDNORDER*sizeof(float));

	/* Diffuser section */

	diffscale = (float)p->fdnlens[3]/(210+159+562+410);
	spread1 = spread;
	spread2 = 3.0*spread;

	b = 210;
	r = 0.125541f;
	a = (int)(spread1*r);
	c = 210+159+a;
	cc = c-b;
	r = 0.854046f;
	a = (int)(spread2*r);
	d = 210+159+562+a;
	dd = d-c;
	e = 1341-d;

	p->ldifs = (ty_diffuser **)malloc(4*sizeof(ty_diffuser *));

	p->ldifs[0] = diffuser_make((int)(diffscale*b),0.75);
	p->ldifs[1] = diffuser_make((int)(diffscale*cc),0.75);
	p->ldifs[2] = diffuser_make((int)(diffscale*dd),0.625);
	p->ldifs[3] = diffuser_make((int)(diffscale*e),0.625);

	b = 210;
	r = -0.568366f;
	a = (int)(spread1*r);
	c = 210+159+a;
	cc = c-b;
	r = -0.126815f;
	a = (int)(spread2*r);
	d = 210+159+562+a;
	dd = d-c;
	e = 1341-d;

	p->rdifs = (ty_diffuser **)malloc(4*sizeof(ty_diffuser *));

	p->rdifs[0] = diffuser_make((int)(diffscale*b),0.75);
	p->rdifs[1] = diffuser_make((int)(diffscale*cc),0.75);
	p->rdifs[2] = diffuser_make((int)(diffscale*dd),0.625);
	p->rdifs[3] = diffuser_make((int)(diffscale*e),0.625);

	/* Tapped delay section */

	p->tapdelay = fixeddelay_make(44000);
	p->taps = (int *)malloc(FDNORDER*sizeof(int));
	p->tapgains = (float *)malloc(FDNORDER*sizeof(float));

	p->taps[0] = (int)(5+0.410*p->largestdelay);
	p->taps[1] = (int)(5+0.300*p->largestdelay);
	p->taps[2] = (int)(5+0.155*p->largestdelay);
	p->taps[3] = (int)(5+0.000*p->largestdelay);

	for(i = 0; i < FDNORDER; i++)
	{
		p->tapgains[i] = pow(p->alpha,(double)p->taps[i]);
	}
	
    }

    // for Chugins extending UGen
    void tick( SAMPLE * in, SAMPLE * out, int nframes )
    {
      float rev[2];
      memset(out, 0, sizeof(SAMPLE)*2*nframes);

      for (int i=0; i < nframes; i+=2)
	{
	  gverb_do(p, in[i], rev, rev+1);
	  
	  out[i] = rev[0] + in[i] * p->drylevel;
	  out[i+1] = rev[1] + in[i+1] * p->drylevel;
	}
	}
  
  // set parameter example
  float setRoomsize( t_CKFLOAT x )
  {
	gverb_set_roomsize(p, x);
	return x;
  }
  
  // get parameter example
  float getRoomsize() { return p->roomsize; }

  // set parameter example
  float setRevTime( t_CKFLOAT x )
  {
	gverb_set_revtime(p, x/p->rate);
	return x;
  }
  
  // get parameter example
  float getRevTime() { return (p->revtime*p->rate); }

  float setDamping( t_CKFLOAT x )
  {
	gverb_set_damping(p, x);
	return x;
  }
  
  // get parameter example
  float getDamping() { return p->fdndamping; }

  float setBandwidth( t_CKFLOAT x )
  {
	gverb_set_inputbandwidth(p, x);
	return x;
  }
  
  // get parameter example
  float getBandwidth() { return p->inputbandwidth; }

  float setDryLevel( t_CKFLOAT x )
  {
	gverb_set_drylevel(p, x);
	return x;
  }
  
  // get parameter example
  float getDryLevel() { return p->drylevel; }

  float setEarlyLevel( t_CKFLOAT x )
  {
	gverb_set_earlylevel(p, x);
	return x;
  }
  
  // get parameter example
  float getEarlyLevel() { return p->earlylevel; }

  float setTailLevel( t_CKFLOAT x )
  {
	gverb_set_taillevel(p, x);
	return x;
  }
  
  // get parameter example
  float getTailLevel() { return p->taillevel; }

private:
  // instance data
  ty_gverb realp;
  ty_gverb *p;
};


// query function: chuck calls this when loading the Chugin
// NOTE: developer will need to modify this function to
// add additional functions to this Chugin
CK_DLL_QUERY( GVerb )
{
    // hmm, don't change this...
    QUERY->setname(QUERY, "GVerb");
    
    // begin the class definition
    // can change the second argument to extend a different ChucK class
    QUERY->begin_class(QUERY, "GVerb", "UGen");

    // register the constructor (probably no need to change)
    QUERY->add_ctor(QUERY, gverb_ctor);
    // register the destructor (probably no need to change)
    QUERY->add_dtor(QUERY, gverb_dtor);

    QUERY->doc_class(QUERY, "GVerb is a very smooth reverberator with the "
                     "ability to produce very long "
                     "reverb times.\n"

                     "GVERB is based on the original \"gverb/gigaverb\" by Juhana Sadeharju "
                     "(kouhia at nic.funet.fi). The code for this version was adapted from "
                     "RTcmix (http:rtcmix.org), which in turn adapted it from the Max/MSP "
                     "version by Olaf Mtthes (olaf.matthes at gmx.de).");
    QUERY->add_ex(QUERY, "effects/GVerb.ck");
    
    // for UGen's only: add tick function
    QUERY->add_ugen_funcf(QUERY, gverb_tick, NULL, 2, 2);
    
    // NOTE: if this is to be a UGen with more than 1 channel, 
    // e.g., a multichannel UGen -- will need to use add_ugen_funcf()
    // and declare a tickf function using CK_DLL_TICKF

    // example of adding setter method
    QUERY->add_mfun(QUERY, gverb_setRoomsize, "float", "roomsize");
    // example of adding argument to the above method
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "Set room size [1.0 - 300.0]. Default 30.0.");

    // example of adding getter method
    QUERY->add_mfun(QUERY, gverb_getRoomsize, "float", "roomsize");
    QUERY->doc_func(QUERY, "Get room size [1.0 - 300.0].");

    // example of adding setter method
    QUERY->add_mfun(QUERY, gverb_setRevTime, "dur", "revtime");
    // example of adding argument to the above method
    QUERY->add_arg(QUERY, "dur", "arg");
    QUERY->doc_func(QUERY, "Set reverberation time. Default 5::second.");

    // example of adding getter method
    QUERY->add_mfun(QUERY, gverb_getRevTime, "dur", "revtime");
    QUERY->doc_func(QUERY, "Get reverberation time.");

    // example of adding getter method
    QUERY->add_mfun(QUERY, gverb_getDamping, "float", "damping");
    QUERY->doc_func(QUERY, "Get high frequency rolloff [0 - 1]. "
                    "0 damps the reverb signal completely, 1 not at all. Default 0.");

    // example of adding setter method
    QUERY->add_mfun(QUERY, gverb_setDamping, "float", "damping");
    // example of adding argument to the above method
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "Set high frequency rolloff [0 - 1]. "
                    "0 damps the reverb signal completely, 1 not at all. Default 0.");

    // example of adding getter method
    QUERY->add_mfun(QUERY, gverb_getBandwidth, "float", "bandwidth");
    QUERY->doc_func(QUERY, "Get the input bandwidth [0 - 1]. "
                    "Same as damping control, but on the input signal. Default 0.5.");

    // example of adding setter method
    QUERY->add_mfun(QUERY, gverb_setBandwidth, "float", "bandwidth");
    // example of adding argument to the above method
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "Set the input bandwidth [0 - 1]. "
                    "Same as damping control, but on the input signal. Default 0.5.");

    // example of adding getter method
    QUERY->add_mfun(QUERY, gverb_getDryLevel, "float", "dry");
    QUERY->doc_func(QUERY, "Get the amount of dry signal [0 - 1]. Default 0.5.");

    // example of adding setter method
    QUERY->add_mfun(QUERY, gverb_setDryLevel, "float", "dry");
    // example of adding argument to the above method
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "Set the amount of dry signal [0 - 1]. Default 0.5.");

    // example of adding getter method
    QUERY->add_mfun(QUERY, gverb_getEarlyLevel, "float", "early");
    QUERY->doc_func(QUERY, "Get the early reflection level [0 - 1]. Default 0.4.");

    // example of adding setter method
    QUERY->add_mfun(QUERY, gverb_setEarlyLevel, "float", "early");
    // example of adding argument to the above method
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "Set the early reflection level [0 - 1]. Default 0.4.");

    // example of adding getter method
    QUERY->add_mfun(QUERY, gverb_getTailLevel, "float", "tail");
    QUERY->doc_func(QUERY, "Get the tail level [0 - 1]. Default 0.5.");

    // example of adding setter method
    QUERY->add_mfun(QUERY, gverb_setTailLevel, "float", "tail");
    // example of adding argument to the above method
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "Set the tail level [0 - 1]. Default 0.5.");
    
    // this reserves a variable in the ChucK internal class to store 
    // referene to the c++ class we defined above
    gverb_data_offset = QUERY->add_mvar(QUERY, "int", "@gv_data", false);

    // end the class definition
    // IMPORTANT: this MUST be called!
    QUERY->end_class(QUERY);

    // wasn't that a breeze?
    return TRUE;
}


// implementation for the constructor
CK_DLL_CTOR(gverb_ctor)
{
    // get the offset where we'll store our internal c++ class pointer
    OBJ_MEMBER_INT(SELF, gverb_data_offset) = 0;
    
    // instantiate our internal c++ class representation
    GVerb * bcdata = new GVerb(API->vm->srate(VM));
    
    // store the pointer in the ChucK object member
    OBJ_MEMBER_INT(SELF, gverb_data_offset) = (t_CKINT) bcdata;
}


// implementation for the destructor
CK_DLL_DTOR(gverb_dtor)
{
    // get our c++ class pointer
    GVerb * bcdata = (GVerb *) OBJ_MEMBER_INT(SELF, gverb_data_offset);
    // check it
    if( bcdata )
    {
        // clean up
        delete bcdata;
        OBJ_MEMBER_INT(SELF, gverb_data_offset) = 0;
        bcdata = NULL;
    }
}


// implementation for tick function
CK_DLL_TICKF(gverb_tick)
{
    // get our c++ class pointer
    GVerb * c = (GVerb *) OBJ_MEMBER_INT(SELF, gverb_data_offset);
 
    // invoke our tick function; store in the magical out variable
    if(c) c->tick(in,out, nframes);

    // yes
    return TRUE;
}


// example implementation for setter
CK_DLL_MFUN(gverb_setRoomsize)
{
    // get our c++ class pointer
    GVerb * bcdata = (GVerb *) OBJ_MEMBER_INT(SELF, gverb_data_offset);
    // set the return value
    RETURN->v_float = bcdata->setRoomsize(GET_NEXT_FLOAT(ARGS));
}


// example implementation for getter
CK_DLL_MFUN(gverb_getRoomsize)
{
    // get our c++ class pointer
    GVerb * bcdata = (GVerb *) OBJ_MEMBER_INT(SELF, gverb_data_offset);
    // set the return value
    RETURN->v_float = bcdata->getRoomsize();
}

// example implementation for setter
CK_DLL_MFUN(gverb_setRevTime)
{
    // get our c++ class pointer
    GVerb * bcdata = (GVerb *) OBJ_MEMBER_INT(SELF, gverb_data_offset);
    // set the return value
    RETURN->v_dur = bcdata->setRevTime(GET_NEXT_DUR(ARGS));
}


// example implementation for getter
CK_DLL_MFUN(gverb_getRevTime)
{
    // get our c++ class pointer
    GVerb * bcdata = (GVerb *) OBJ_MEMBER_INT(SELF, gverb_data_offset);
    // set the return value
    RETURN->v_float = bcdata->getRevTime();
}

// example implementation for setter
CK_DLL_MFUN(gverb_setDamping)
{
    // get our c++ class pointer
    GVerb * bcdata = (GVerb *) OBJ_MEMBER_INT(SELF, gverb_data_offset);
    // set the return value
    RETURN->v_float = bcdata->setDamping(GET_NEXT_FLOAT(ARGS));
}


// example implementation for getter
CK_DLL_MFUN(gverb_getDamping)
{
    // get our c++ class pointer
    GVerb * bcdata = (GVerb *) OBJ_MEMBER_INT(SELF, gverb_data_offset);
    // set the return value
    RETURN->v_float = bcdata->getDamping();
}

// example implementation for setter
CK_DLL_MFUN(gverb_setBandwidth)
{
    // get our c++ class pointer
    GVerb * bcdata = (GVerb *) OBJ_MEMBER_INT(SELF, gverb_data_offset);
    // set the return value
    RETURN->v_float = bcdata->setBandwidth(GET_NEXT_FLOAT(ARGS));
}


// example implementation for getter
CK_DLL_MFUN(gverb_getBandwidth)
{
    // get our c++ class pointer
    GVerb * bcdata = (GVerb *) OBJ_MEMBER_INT(SELF, gverb_data_offset);
    // set the return value
    RETURN->v_float = bcdata->getBandwidth();
}

// example implementation for setter
CK_DLL_MFUN(gverb_setDryLevel)
{
    // get our c++ class pointer
    GVerb * bcdata = (GVerb *) OBJ_MEMBER_INT(SELF, gverb_data_offset);
    // set the return value
    RETURN->v_float = bcdata->setDryLevel(GET_NEXT_FLOAT(ARGS));
}


// example implementation for getter
CK_DLL_MFUN(gverb_getDryLevel)
{
    // get our c++ class pointer
    GVerb * bcdata = (GVerb *) OBJ_MEMBER_INT(SELF, gverb_data_offset);
    // set the return value
    RETURN->v_float = bcdata->getDryLevel();
}

// example implementation for setter
CK_DLL_MFUN(gverb_setEarlyLevel)
{
    // get our c++ class pointer
    GVerb * bcdata = (GVerb *) OBJ_MEMBER_INT(SELF, gverb_data_offset);
    // set the return value
    RETURN->v_float = bcdata->setEarlyLevel(GET_NEXT_FLOAT(ARGS));
}


// example implementation for getter
CK_DLL_MFUN(gverb_getEarlyLevel)
{
    // get our c++ class pointer
    GVerb * bcdata = (GVerb *) OBJ_MEMBER_INT(SELF, gverb_data_offset);
    // set the return value
    RETURN->v_float = bcdata->getEarlyLevel();
}

// example implementation for setter
CK_DLL_MFUN(gverb_setTailLevel)
{
    // get our c++ class pointer
    GVerb * bcdata = (GVerb *) OBJ_MEMBER_INT(SELF, gverb_data_offset);
    // set the return value
    RETURN->v_float = bcdata->setTailLevel(GET_NEXT_FLOAT(ARGS));
}


// example implementation for getter
CK_DLL_MFUN(gverb_getTailLevel)
{
    // get our c++ class pointer
    GVerb * bcdata = (GVerb *) OBJ_MEMBER_INT(SELF, gverb_data_offset);
    // set the return value
    RETURN->v_float = bcdata->getTailLevel();
}
