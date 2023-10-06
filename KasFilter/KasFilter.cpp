/*----------------------------------------------------------------------------
    "Kas-filter" ChuGin for ChucK

    Copyright (c) 2011 Kassen Oud.
      signal.automatique@gmail.com

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


/*-----------------------------------------------------------------------------
"kasfilter";
Under-sampling-based resonant lowpass filter, 
based on two sample & holds with a cosine crossfading between them. Each S&H
samples at the moment it's faded out.
The frequency of the crossfading and sampling of the input sets the cutoff.
This leads to a infinitely steep cutoff, at the price of aliasing.
Negative feedback is used for resonance at the cutoff frequency, a technique
that I believe is new here.
In addition to the traditional modulation options waveshaping of the crossfading 
signal is provided. This leads to distortion at the cutoff frequency. 
At extreme values (and assuming no feedback is used) that makes the effect closer
to traditional under-sampling.
Linear interpolation is used on the input signal to avoid the S&Hs 
being quantised to ChucK's sample rate; This technique causes quite a few 
interesting artifacts that result from the ratio between the frequency of the 
input and the cutoff frequency. To emphasise those it makes sense to try to 
minimise artifacts induced by the digital environment itself. Thanks to 
Rob Hordijk for sharing his insights there.
-----------------------------------------------------------------------------*/
#include "chuck_dl.h"
#include "chuck_def.h"

#include <stdio.h>
#include <limits.h>
#include <math.h> //because we need fabs


CK_DLL_CTOR(kasfilter_ctor);
CK_DLL_DTOR(kasfilter_dtor);

CK_DLL_MFUN(kasfilter_setFreq);
CK_DLL_MFUN(kasfilter_getFreq);
CK_DLL_MFUN(kasfilter_setResonance);
CK_DLL_MFUN(kasfilter_getResonance);
CK_DLL_MFUN(kasfilter_setAccent);
CK_DLL_MFUN(kasfilter_getAccent);

CK_DLL_TICK(kasfilter_tick);

t_CKINT kasfilter_data_offset = 0;


struct KasFilterData
{
    float freq;
    float resonance;
	float accent;
	float storeA;
	float storeB;
	float lastIn;
	float phase;
	float PhasePerSample;
};


CK_DLL_QUERY(KasFilter)
{
    QUERY->setname(QUERY, "KasFilter");
    
    QUERY->begin_class(QUERY, "KasFilter", "UGen");
    
    QUERY->add_ctor(QUERY, kasfilter_ctor);
    QUERY->add_dtor(QUERY, kasfilter_dtor);

    QUERY->doc_class(QUERY, "Under-sampling-based resonant lowpass filter, "
                     "based on two sample & holds with a cosine crossfading between them. Each S&H "
                     "samples at the moment it's faded out.\n"
                     "The frequency of the crossfading and sampling of the input sets the cutoff. "
                     "This leads to a infinitely steep cutoff, at the price of aliasing. "
                     "Negative feedback is used for resonance at the cutoff frequency, a technique "
                     "that I believe is new here.\n"
                     "In addition to the traditional modulation options waveshaping of "
                     "the crossfading "
                     "signal is provided. This leads to distortion at the cutoff frequency. "
                     "At extreme values (and assuming no feedback is used) that makes the "
                     "effect closer "
                     "to traditional under-sampling.\n"
                     "Linear interpolation is used on the input signal to avoid the S&Hs "
                     "being quantised to ChucK's sample rate; This technique causes quite a few "
                     "interesting artifacts that result from the ratio between the frequency of the "
                     "input and the cutoff frequency. To emphasise those it makes sense to try to "
                     "minimise artifacts induced by the digital environment itself. Thanks to "
                     "Rob Hordijk for sharing his insights there.");
    QUERY->add_ex(QUERY, "filter/KasFilter.ck");
    
    QUERY->add_ugen_func(QUERY, kasfilter_tick, NULL, 1, 1);
    
    QUERY->add_mfun(QUERY, kasfilter_setFreq, "float", "freq");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "Sets the cutoff frequency. "
                    "This sets both the frequency at which the two "
                    "sample & holds sample the input signal "
                    " and the frequency of the sine that crossfades between them.");

    QUERY->add_mfun(QUERY, kasfilter_getFreq, "float", "freq");
    QUERY->doc_func(QUERY, "Gets the cutoff frequency. "
                    "This sets both the frequency at which the two "
                    "sample & holds sample the input signal "
                    " and the frequency of the sine that crossfades between them.");


    QUERY->add_mfun(QUERY, kasfilter_setResonance, "float", "resonance");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "Sets the resonance, which is implemented "
                    "as negative feedback [0 - 0.95].");

    QUERY->add_mfun(QUERY, kasfilter_getResonance, "float", "resonance");
    QUERY->doc_func(QUERY, "Gets the resonance, which is implemented "
                    "as negative feedback [0 - 0.95].");

    QUERY->add_mfun(QUERY, kasfilter_setAccent, "float", "accent");
    QUERY->add_arg(QUERY, "float", "arg");
    QUERY->doc_func(QUERY, "Sets the amount of waveshaping on the crossfading sine [0 - 1]. "
                    "1 is close to regular under-sampling (if no resonance is used).");
    
    QUERY->add_mfun(QUERY, kasfilter_getAccent, "float", "accent");
    QUERY->doc_func(QUERY, "Gets the amount of waveshaping on the crossfading sine [0 - 1]. "
                    "1 is close to regular under-sampling (if no resonance is used).");

    kasfilter_data_offset = QUERY->add_mvar(QUERY, "int", "@data", false);
    
    QUERY->end_class(QUERY);

    return TRUE;
}


CK_DLL_CTOR(kasfilter_ctor)
{
    OBJ_MEMBER_INT(SELF, kasfilter_data_offset) = 0;
    
    KasFilterData * kfdata = new KasFilterData;
    kfdata->PhasePerSample	= CK_ONE_PI / (t_CKUINT)API->vm->srate(VM);
    kfdata->freq			= 440;
    kfdata->resonance			= 0;
    kfdata->accent			= 0;
    kfdata->storeA			= 0;
    kfdata->storeB			= 0;
    kfdata->lastIn			= 0;
    kfdata->phase			= 0;
    
    OBJ_MEMBER_INT(SELF, kasfilter_data_offset) = (t_CKINT) kfdata;
}

CK_DLL_DTOR(kasfilter_dtor)
{
    KasFilterData * kfdata = (KasFilterData *) OBJ_MEMBER_INT(SELF, kasfilter_data_offset);
    if(kfdata)
    {
        delete kfdata;
        OBJ_MEMBER_INT(SELF, kasfilter_data_offset) = 0;
        kfdata = NULL;
    }
}

CK_DLL_TICK(kasfilter_tick)
{
    KasFilterData * kfdata = (KasFilterData *) OBJ_MEMBER_INT(SELF, kasfilter_data_offset);

	float lastPhase = kfdata->phase;
	if (kfdata->freq > 0)
	{
		float PhaseInc = kfdata->PhasePerSample * kfdata->freq;
		kfdata->phase += PhaseInc;

		if (kfdata->phase > CK_TWO_PI) //sample the input at the exact extremes of the crossfading wave
		{
			kfdata->phase -= (float)CK_TWO_PI;
			float interp = kfdata->phase / PhaseInc; //this division should be safe; PhaseInc should never be 0 at this moment
			kfdata->storeB = (in * interp) + (kfdata->lastIn * (1 - interp)); //interpolate based on how far we overshot the extreme of the wave.
			kfdata->storeB += (kfdata->resonance * kfdata->storeA); //apply feedback.
			kfdata->storeB = ck_max (-1.0f ,  ck_min ( 1.0f , kfdata->storeB)); //clamp because if we don't it'll build up indefinitely at certain inputs. Thanks to the x-fading the eventual output won't hard-clip.
		}
		else if (kfdata->phase > CK_ONE_PI && lastPhase < CK_ONE_PI) // and again for the other s&h
		{
			float interp = (kfdata->phase - CK_ONE_PI) / PhaseInc;
			kfdata->storeA =  ( in * interp) + (kfdata->lastIn * (1 - interp)); 
			kfdata->storeA += (kfdata->resonance * kfdata->storeB); 
			kfdata->storeA = ck_max (-1.0f ,  ck_min ( 1.0f , kfdata->storeA)); 
		}
	}
	float mix = cos(kfdata->phase);
	float absmix = fabs(mix);
		
	mix = 0.5f + (0.5f * mix * (absmix + ( kfdata->accent * (1 - absmix)))); //apply wave-shaping, then get signal in 0-1 range
	*out = (kfdata->storeA * mix) + (kfdata->storeB * (1 - mix)); //actual crossfading
	kfdata->lastIn=in;
 

    return TRUE;
}

CK_DLL_MFUN(kasfilter_setFreq)
{
    KasFilterData * kfdata = (KasFilterData *) OBJ_MEMBER_INT(SELF, kasfilter_data_offset);
    float amnt = GET_NEXT_FLOAT(ARGS);
	amnt = fabs(amnt);							//negative frequencies make no sense here.
    kfdata->freq = amnt;
    RETURN->v_float = kfdata->freq;				//I'm joining Spencer in making such functions return their input; useful for daisy-chaining.
}

CK_DLL_MFUN(kasfilter_getFreq)
{
    KasFilterData * kfdata = (KasFilterData *) OBJ_MEMBER_INT(SELF, kasfilter_data_offset);
    RETURN->v_float = kfdata->freq;			
}

CK_DLL_MFUN(kasfilter_setResonance)
{
    KasFilterData * kfdata = (KasFilterData *) OBJ_MEMBER_INT(SELF, kasfilter_data_offset);
    t_CKFLOAT amnt = GET_NEXT_FLOAT(ARGS);
	if (amnt < 0) amnt = 0;
	else if (amnt > 0.95) amnt = 0.95;			 //because otherwise things get a bit out of hand
    kfdata->resonance = amnt * -1;				 //negative feedback for oscillation instead of buildup
    RETURN->v_float =  amnt;
}

CK_DLL_MFUN(kasfilter_getResonance)
{
    KasFilterData * kfdata = (KasFilterData *) OBJ_MEMBER_INT(SELF, kasfilter_data_offset);
    RETURN->v_float = kfdata->resonance * -1;
}

CK_DLL_MFUN(kasfilter_setAccent)
{
    KasFilterData * kfdata = (KasFilterData *) OBJ_MEMBER_INT(SELF, kasfilter_data_offset);
    float amnt = GET_NEXT_FLOAT(ARGS);
	if (amnt > 1) amnt = 1;
	else if (amnt < 0) amnt = 0;
    kfdata->accent = amnt + 1;
    RETURN->v_float = amnt;
}

CK_DLL_MFUN(kasfilter_getAccent)
{
    KasFilterData * kfdata = (KasFilterData *) OBJ_MEMBER_INT(SELF, kasfilter_data_offset);
    RETURN->v_float = kfdata->accent - 1;
}
