/* Copyright (c) 2005 Miller Puckette.  BSD licensed.  No warranties. */

#include "Sigmund.h"

// general includes
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>

int sigmund_ilog2(int n)
{
  int ret = -1;
  while (n)
    {
      n >>= 1;
      ret++;
    }
  return (ret);
}

t_float sigmund_ftom(t_float f)
{
  return (f > 0 ? 17.3123405046 * log(.12231220585 * f) : -1500);
}

#define LOGTEN 2.302585092994
t_float sigmund_powtodb(t_float f)
{
  if (f <= 0) return (0);
  else
    {
      t_float val = 100 + 10./LOGTEN * log(f);
      return (val < 0 ? 0 : val);
    }
}

/* parameters for von Hann window (change these to get Hamming if desired) */
#define W_ALPHA 0.5
#define W_BETA 0.5
#define NEGBINS 4   /* number of bins of negative frequency we'll need */
  
#define PI 3.141592653589793
#define LOG2  0.693147180559945
  
  t_float sinx(t_float theta, t_float sintheta)
  {
    if (theta > -0.003 && theta < 0.003)
      return (1);
    else return (sintheta/theta);
  }
  
  t_float window_hann_mag(t_float pidetune, t_float sinpidetune)
  {
    return (W_ALPHA * sinx(pidetune, sinpidetune)
	    - 0.5 * W_BETA *
            (sinx(pidetune+PI, sinpidetune) + sinx(pidetune-PI, sinpidetune)));
  }
  
  t_float window_mag(t_float pidetune, t_float cospidetune)
  {
    return (sinx(pidetune + (PI/2), cospidetune)
	    + sinx(pidetune - (PI/2), -cospidetune));
  }
  
  /*********** Routines to analyze a window into sinusoidal peaks *************/
  
  int sigmund_cmp_freq(const void *p1, const void *p2)
  {
    if ((*(t_peak **)p1)->p_freq > (*(t_peak **)p2)->p_freq)
      return (1);
    else if ((*(t_peak **)p1)->p_freq < (*(t_peak **)p2)->p_freq)
      return (-1);
    else return (0);
  }
  
  void sigmund_tweak(int npts, t_float *ftreal, t_float *ftimag,
			    int npeak, t_peak *peaks, t_float fperbin, int loud)
  {
    t_peak **peakptrs = (t_peak **)alloca(sizeof (*peakptrs) * (npeak+1));
    t_peak negpeak;
    int peaki, j, k;
    t_float ampreal[3], ampimag[3];
    t_float binperf = 1./fperbin;
    t_float phaseperbin = (npts-0.5)/npts, oneovern = 1./npts;
    if (npeak < 1)
      return;
    for (peaki = 0; peaki < npeak; peaki++)
      peakptrs[peaki+1] = &peaks[peaki];
    qsort(peakptrs+1, npeak, sizeof (*peakptrs), sigmund_cmp_freq);
    peakptrs[0] = &negpeak;
    negpeak.p_ampreal = peakptrs[1]->p_ampreal;
    negpeak.p_ampimag = -peakptrs[1]->p_ampimag;
    negpeak.p_freq = -peakptrs[1]->p_freq;
    for (peaki = 1; peaki <= npeak; peaki++)
      {
        int cbin = peakptrs[peaki]->p_freq*binperf + 0.5;
        int nsub = (peaki == npeak ? 1:2);
        t_float windreal, windimag, windpower, detune, pidetune, sinpidetune,
	  cospidetune, ampcorrect, ampout, ampoutreal, ampoutimag, freqout;
        /* post("3 nsub %d amp %f freq %f", nsub,
	   peakptrs[peaki]->p_amp, peakptrs[peaki]->p_freq); */
        if (cbin < 0 || cbin > 2*npts - 3)
	  continue;
        for (j = 0; j < 3; j++)
	  ampreal[j] = ftreal[cbin+2*j-2], ampimag[j] = ftimag[cbin+2*j-2];
        /* post("a %f %f", ampreal[1], ampimag[1]); */
        for (j = 0; j < nsub; j++)
	  {
            t_peak *neighbor = peakptrs[(peaki-1) + 2*j];
            t_float neighborreal = npts * neighbor->p_ampreal;
            t_float neighborimag = npts * neighbor->p_ampimag;
            for (k = 0; k < 3; k++)
	      {
                t_float freqdiff = (0.5*PI) * ((cbin + 2*k-2)
					       -binperf * neighbor->p_freq);
                t_float sx = sinx(freqdiff, sin(freqdiff));
                t_float phasere = cos(freqdiff * phaseperbin);
                t_float phaseim = sin(freqdiff * phaseperbin);
                ampreal[k] -=
		  sx * (phasere * neighborreal - phaseim * neighborimag);
                ampimag[k] -=
		  sx * (phaseim * neighborreal + phasere * neighborimag);
	      }       
            /* post("b %f %f", ampreal[1], ampimag[1]); */
	  }
	
        windreal = W_ALPHA * ampreal[1] -
	  (0.5 * W_BETA) * (ampreal[0] + ampreal[2]);
        windimag = W_ALPHA * ampimag[1] -
	  (0.5 * W_BETA) * (ampimag[0] + ampimag[2]);
        windpower = windreal * windreal + windimag * windimag;
        detune = (
		  W_BETA*(ampreal[0] - ampreal[2]) * 
		  (2.0*W_ALPHA * ampreal[1] - W_BETA * (ampreal[0] + ampreal[2]))
		  +
		  W_BETA*(ampimag[0] - ampimag[2]) *
		  (2.0*W_ALPHA * ampimag[1] - W_BETA * (ampimag[0] + ampimag[2]))
		  ) / (4.0 * windpower);
        if (detune > 0.5)
	  detune = 0.5;
        else if (detune < -0.5)
	  detune = -0.5;
        /* if (loud > 0)
	   post("tweak: windpower %f, bin %d, detune %f",
	   windpower, cbin, detune); */
        pidetune = PI * detune;
        sinpidetune = sin(pidetune);
        cospidetune = cos(pidetune);
	
        ampcorrect = 1.0 / window_hann_mag(pidetune, sinpidetune);
	
        ampout = oneovern * ampcorrect *sqrt(windpower);
        ampoutreal = oneovern * ampcorrect *
	  (windreal * cospidetune - windimag * sinpidetune);
        ampoutimag = oneovern * ampcorrect *
	  (windreal * sinpidetune + windimag * cospidetune);
        freqout = (cbin + 2*detune) * fperbin;
        /* if (loud > 1)
	   post("amp %f, freq %f", ampout, freqout); */
        
        peakptrs[peaki]->p_freq = freqout;
        peakptrs[peaki]->p_amp = ampout;
        peakptrs[peaki]->p_ampreal = ampoutreal;
        peakptrs[peaki]->p_ampimag = ampoutimag;
      }
  }
  
  void sigmund_remask(int maxbin, int bestindex, t_float powmask, 
			     t_float maxpower, t_float *maskbuf)
  {
    int bin;
    int bin1 = (bestindex > 52 ? bestindex-50:2);
    int bin2 = (maxbin < bestindex + 50 ? bestindex + 50 : maxbin);
    for (bin = bin1; bin < bin2; bin++)
      {
        t_float bindiff = bin - bestindex;
        t_float mymask;
        mymask = powmask/ (1. + bindiff * bindiff * bindiff * bindiff);
        if (bindiff < 2 && bindiff > -2)
	  mymask = 2*maxpower;
        if (mymask > maskbuf[bin])
	  maskbuf[bin] = mymask;
      } 
  }
  
#define PEAKMASKFACTOR 1.
#define PEAKTHRESHFACTOR 0.6
  
  void sigmund_getrawpeaks(int npts, t_float *insamps,
				  int npeak, t_peak *peakv, int *nfound, t_float *power, t_float srate, int loud,
				  t_float hifreq)
  {
    t_float oneovern = 1.0/ (t_float)npts;
    t_float fperbin = 0.5 * srate * oneovern, totalpower = 0;
    int npts2 = 2*npts, i, bin;
    int peakcount = 0;
    t_float *fp1, *fp2;
    t_float *rawreal, *rawimag, *maskbuf, *powbuf;
    t_float *bigbuf = (t_float*)alloca(sizeof (t_float ) * (2*NEGBINS + 6*npts));
    int maxbin = hifreq/fperbin;
    if (maxbin > npts - NEGBINS)
      maxbin = npts - NEGBINS;
    /* if (loud) post("tweak %d", tweak); */
    maskbuf = bigbuf + npts2;
    powbuf = maskbuf + npts;
    rawreal = powbuf + npts+NEGBINS;
    rawimag = rawreal+npts+NEGBINS;
    for (i = 0; i < npts; i++)
      maskbuf[i] = 0;
    
    for (i = 0; i < npts; i++)
      bigbuf[i] = insamps[i];
    for (i = npts; i < 2*npts; i++)
      bigbuf[i] = 0;
    mayer_realfft(npts2, bigbuf);
    for (i = 0; i < npts; i++)
      rawreal[i] = bigbuf[i];
    for (i = 1; i < npts-1; i++)
      rawimag[i] = bigbuf[npts2-i];
    rawreal[-1] = rawreal[1];
    rawreal[-2] = rawreal[2];
    rawreal[-3] = rawreal[3];
    rawreal[-4] = rawreal[4];
    rawimag[0] = rawimag[npts-1] = 0;
    rawimag[-1] = -rawimag[1];
    rawimag[-2] = -rawimag[2];
    rawimag[-3] = -rawimag[3];
    rawimag[-4] = -rawimag[4];
#if 1
    for (i = 0, fp1 = rawreal, fp2 = rawimag; i < maxbin; i++, fp1++, fp2++)
      {
        t_float x1 = fp1[1] - fp1[-1], x2 = fp2[1] - fp2[-1], p = powbuf[i] = x1*x1+x2*x2; 
        if (i >= 2)
	  totalpower += p;
      }
    powbuf[maxbin] = powbuf[maxbin+1] = 0;
    *power = 0.5 * totalpower *oneovern * oneovern;
#endif
    for (peakcount = 0; peakcount < npeak; peakcount++)
      {
        t_float pow1, maxpower = 0, windreal, windimag, windpower,
	  detune, pidetune, sinpidetune, cospidetune, ampcorrect, ampout,
	  ampoutreal, ampoutimag, freqout, powmask;
        int bestindex = -1;
	
        for (bin = 2, fp1 = rawreal+2, fp2 = rawimag+2;
	     bin < maxbin; bin++, fp1++, fp2++)
	  {
            pow1 = powbuf[bin];
            if (pow1 > maxpower && pow1 > maskbuf[bin])
	      {
                t_float thresh = PEAKTHRESHFACTOR * (powbuf[bin-2]+powbuf[bin+2]);
                if (pow1 > thresh)
		  maxpower = pow1, bestindex = bin;
	      }
	  }
	
        if (totalpower <= 0 || maxpower < 1e-10*totalpower || bestindex < 0)
	  break;
        fp1 = rawreal+bestindex;
        fp2 = rawimag+bestindex;
        powmask = maxpower * PEAKMASKFACTOR;
        /* if (loud > 2)
	   post("maxpower %f, powmask %f, param1 %f",
	   maxpower, powmask, param1); */
        sigmund_remask(maxbin, bestindex, powmask, maxpower, maskbuf);
        
        /* if (loud > 1)
	   post("best index %d, total power %f", bestindex, totalpower); */
	
        windreal = fp1[1] - fp1[-1];
        windimag = fp2[1] - fp2[-1];
        windpower = windreal * windreal + windimag * windimag;
        detune = ((fp1[1] * fp1[1] - fp1[-1]*fp1[-1]) 
		  + (fp2[1] * fp2[1] - fp2[-1]*fp2[-1])) / (2 * windpower);
	
        if (detune > 0.5)
	  detune = 0.5;
        else if (detune < -0.5)
	  detune = -0.5;
        /* if (loud > 1)
	   post("windpower %f, index %d, detune %f",
	   windpower, bestindex, detune); */
        pidetune = PI * detune;
        sinpidetune = sin(pidetune);
        cospidetune = cos(pidetune);
        ampcorrect = 1.0 / window_mag(pidetune, cospidetune);
	
        ampout = ampcorrect *sqrt(windpower);
        ampoutreal = ampcorrect *
	  (windreal * cospidetune - windimag * sinpidetune);
        ampoutimag = ampcorrect *
	  (windreal * sinpidetune + windimag * cospidetune);
	
	/* the frequency is the sum of the bin frequency and detuning */
	
        peakv[peakcount].p_freq = (freqout = (bestindex + 2*detune)) * fperbin;
        peakv[peakcount].p_amp = oneovern * ampout;
        peakv[peakcount].p_ampreal = oneovern * ampoutreal;
        peakv[peakcount].p_ampimag = oneovern * ampoutimag;
      }
    sigmund_tweak(npts, rawreal, rawimag, peakcount, peakv, fperbin, loud);
    sigmund_tweak(npts, rawreal, rawimag, peakcount, peakv, fperbin, loud);
    for (i = 0; i < peakcount; i++)
      {
        peakv[i].p_pit = sigmund_ftom(peakv[i].p_freq);
        peakv[i].p_db = sigmund_powtodb(peakv[i].p_amp);
      }
    *nfound = peakcount;
  }
  
  /*************** Routines for finding fundamental pitch *************/
  
#define PITCHNPEAK 12
#define HALFTONEINC 0.059
#define SUBHARMONICS 16
#define DBPERHALFTONE 0.0
  
void sigmund_getpitch(int npeak, t_peak *peakv, t_float *freqp,
		      t_float npts, t_float srate, t_float nharmonics, t_float amppower, int loud)
{
  t_float fperbin = 0.5 * srate / npts;
  int npit = 48 * sigmund_ilog2(npts), i, j, k, nsalient;
  t_float bestbin, bestweight, sumamp, sumweight, sumfreq, freq;
  t_float *weights =  (t_float *)alloca(sizeof(t_float) * npit);
  t_peak *bigpeaks[PITCHNPEAK];
  if (npeak < 1)
    {
      freq = 0;
      goto done;
    }
  for (i = 0; i < npit; i++)
    weights[i] = 0;
  for (i = 0; i < npeak; i++)
    {
      peakv[i].p_tmp = 0;
      peakv[i].p_salience = peakv[i].p_db - DBPERHALFTONE * peakv[i].p_pit;
    }
  for (nsalient = 0; nsalient < PITCHNPEAK; nsalient++)
    {
      t_peak *bestpeak = 0;
      t_float bestsalience = -1e20;
      for (j = 0; j < npeak; j++)
	if (peakv[j].p_tmp == 0 && peakv[j].p_salience > bestsalience)
	  {
	    bestsalience = peakv[j].p_salience;
	    bestpeak = &peakv[j];
	  }
      if (!bestpeak)
	break;
      bigpeaks[nsalient] = bestpeak;
      bestpeak->p_tmp = 1;
      /* post("peak f=%f a=%f", bestpeak->p_freq, bestpeak->p_amp); */
    }
  sumweight = 0;
  for (i = 0; i < nsalient; i++)
    {
      t_peak *thispeak = bigpeaks[i];
      t_float weightindex = (48./LOG2) *
	log(thispeak->p_freq/(2.*fperbin));
      t_float loudness = pow(thispeak->p_amp, amppower);
      /* post("index %f, uncertainty %f", weightindex, pitchuncertainty); */
      for (j = 0; j < SUBHARMONICS; j++)
	{
	  t_float subindex = weightindex -
	    (48./LOG2) * log(j + 1.);
	  int loindex = subindex - 0.5;
	  int hiindex = loindex+2;
	  if (hiindex < 0)
	    break;
	  if (hiindex >= npit)
	    continue;
	  if (loindex < 0)
	    loindex = 0;
	  for (k = loindex; k <= hiindex; k++)
	    weights[k] += loudness * nharmonics / (nharmonics + j);
	}
      sumweight += loudness;
    }
  bestbin = -1;
  bestweight = -1e20;
  for (i = 0; i < npit; i++)
    if (weights[i] > bestweight)
      bestweight = weights[i], bestbin = i;
  if (bestweight < sumweight * 0.4)
    bestbin = -1;
  
  if (bestbin < 0)
    {
      freq = 0;
      goto done;
    }
  if (bestbin > 0 && bestbin < npit-1)
    {
      int ibest = bestbin;
      bestbin += (weights[ibest+1] - weights[ibest-1]) /
	(weights[ibest+1] +  weights[ibest] + weights[ibest-1]);
    }
  freq = 2*fperbin * exp((LOG2/48.)*bestbin);
  for (sumamp = sumweight = sumfreq = 0, i = 0; i < nsalient; i++)
    {
      t_peak *thispeak = bigpeaks[i];
      t_float thisloudness = thispeak->p_amp;
      t_float thisfreq = thispeak->p_freq;
      t_float harmonic = thisfreq/freq;
      t_float intpart = (int)(0.5 + harmonic);
      t_float inharm = harmonic - intpart;
#if 0
      if (loud)
	post("freq %f intpart %f inharm %f", freq, intpart, inharm);
#endif
      if (intpart >= 1 && intpart <= 16 &&
	  inharm < 0.015 * intpart && inharm > - (0.015 * intpart))
	{
	  t_float weight = thisloudness * intpart;
	  sumweight += weight;
	  sumfreq += weight*thisfreq/intpart;
#if 0
	  if (loud)
	    post("weight %f freq %f", weight, thisfreq);
#endif
	}
    }
  if (sumweight > 0)
    freq = sumfreq / sumweight;
 done:
  if (!(freq >= 0 || freq <= 0))
    {
      /* post("freq nan cancelled"); */
      freq = 0;
    }
  *freqp = freq;
}

/*************** gather peak lists into sinusoidal tracks *************/

void sigmund_peaktrack(int ninpeak, t_peak *inpeakv, 
		       int noutpeak, t_peak *outpeakv, int loud)
{
  int incnt, outcnt;
  for (outcnt = 0; outcnt < noutpeak; outcnt++)
    outpeakv[outcnt].p_tmp = -1;
  
  /* first pass. Match each "in" peak with the closest previous
     "out" peak, but no two to the same one. */
  for (incnt = 0; incnt < ninpeak; incnt++)
    {
      t_float besterror = 1e20;
      int bestcnt = -1;
      inpeakv[incnt].p_tmp = -1;
      for (outcnt = 0; outcnt < noutpeak; outcnt++)
	{
	  t_float thiserror =
	    inpeakv[incnt].p_freq - outpeakv[outcnt].p_freq;
	  if (thiserror < 0)
	    thiserror = -thiserror;
	  if (thiserror < besterror)
	    {
	      besterror = thiserror;
	      bestcnt = outcnt;
	    }
	}
      if (outpeakv[bestcnt].p_tmp < 0)
	{
	  outpeakv[bestcnt] = inpeakv[incnt];
	  inpeakv[incnt].p_tmp = 0;
	  outpeakv[bestcnt].p_tmp = 0;
	}
    }
  /* second pass.  Unmatched "in" peaks assigned to free "out"
     peaks */
  for (incnt = 0; incnt < ninpeak; incnt++)
    if (inpeakv[incnt].p_tmp < 0)
      {
	for (outcnt = 0; outcnt < noutpeak; outcnt++)
	  if (outpeakv[outcnt].p_tmp < 0)
	    {
	      outpeakv[outcnt] = inpeakv[incnt];
	      inpeakv[incnt].p_tmp = 0;
	      outpeakv[outcnt].p_tmp = 1;
	      break;
	    }
      }
  for (outcnt = 0; outcnt < noutpeak; outcnt++)
    if (outpeakv[outcnt].p_tmp == -1)
      outpeakv[outcnt].p_amp = 0;
}

/**************** parse continuous pitch into note starts ***************/

void notefinder_init(t_notefinder *x)
{
  int i;
  x->n_peaked = x->n_age = 0;
  x->n_hifreq = x->n_lofreq = 0;
  x->n_histphase = 0;
  for (i = 0; i < NHISTPOINT; i++)
    x->n_hist[i].h_freq =x->n_hist[i].h_power = 0;
}

void notefinder_doit(t_notefinder *x, t_float freq, t_float power,
		     t_float *note, t_float vibrato, int stableperiod, t_float powerthresh,
		     t_float growththresh, int loud)
{
  /* calculate frequency ratio between allowable vibrato extremes
     (equal to twice the vibrato deviation from center) */
  t_float vibmultiple = exp((2*LOG2/12) * vibrato);
  int oldhistphase, i, k;
  if (stableperiod > NHISTPOINT - 1)
    stableperiod = NHISTPOINT - 1;
  else if (stableperiod < 1)
    stableperiod = 1;
  if (++x->n_histphase == NHISTPOINT)
    x->n_histphase = 0;
  x->n_hist[x->n_histphase].h_freq = freq;
  x->n_hist[x->n_histphase].h_power = power;
  x->n_age++;
  *note = 0;
#if 0
  if (loud)
    {
      post("stable %d, age %d, vibmultiple %f, powerthresh %f, hifreq %f",
	   stableperiod, (int)x->n_age ,vibmultiple, powerthresh, x->n_hifreq);
      post("histfreq %f %f %f %f",
	   x->n_hist[x->n_histphase].h_freq,
	   x->n_hist[(x->n_histphase+NHISTPOINT-1)%NHISTPOINT].h_freq,
	   x->n_hist[(x->n_histphase+NHISTPOINT-2)%NHISTPOINT].h_freq,
	   x->n_hist[(x->n_histphase+NHISTPOINT-3)%NHISTPOINT].h_freq);
      post("power %f %f %f %f",
	   x->n_hist[x->n_histphase].h_power,
	   x->n_hist[(x->n_histphase+NHISTPOINT-1)%NHISTPOINT].h_power,
	   x->n_hist[(x->n_histphase+NHISTPOINT-2)%NHISTPOINT].h_power,
	   x->n_hist[(x->n_histphase+NHISTPOINT-3)%NHISTPOINT].h_power);
      for (i = 0, k = x->n_histphase; i < stableperiod; i++)
	{
	  post("pit %5.1f  pow %f", sigmund_ftom(x->n_hist[k].h_freq),
	       x->n_hist[k].h_power);
	  if (--k < 0)
	    k = NHISTPOINT - 1;
	}
    }
#endif
  /* look for shorter notes than "stableperiod" in length.
     The amplitude must rise and then fall while the pitch holds
     steady. */
  if (x->n_hifreq <= 0 && x->n_age > stableperiod)
    {
      t_float maxpow = 0, freqatmaxpow = 0,
	localhifreq = -1e20, locallofreq = 1e20;
      int startphase = x->n_histphase - stableperiod + 1;
      if (startphase < 0)
	startphase += NHISTPOINT;
      for (i = 0, k = startphase; i < stableperiod; i++)
	{
	  if (x->n_hist[k].h_freq <= 0)
	    break;
	  if (x->n_hist[k].h_power > maxpow)
	    maxpow = x->n_hist[k].h_power,
	      freqatmaxpow = x->n_hist[k].h_freq;
	  if (x->n_hist[k].h_freq > localhifreq)
	    localhifreq = x->n_hist[k].h_freq;
	  if (x->n_hist[k].h_freq < locallofreq)
	    locallofreq = x->n_hist[k].h_freq;
	  if (localhifreq > locallofreq * vibmultiple)
	    break;
	  if (maxpow > power * growththresh &&
	      maxpow > x->n_hist[startphase].h_power * growththresh &&
	      localhifreq < vibmultiple * locallofreq
	      && freqatmaxpow > 0 && maxpow > powerthresh)
	    {
	      x->n_hifreq = x->n_lofreq = *note = freqatmaxpow;
	      x->n_age = 0;
	      x->n_peaked = 0;
	      /* post("got short note"); */
	      return;
	    }
	  if (++k >= NHISTPOINT)
	    k = 0;
	}
      
    }
  if (x->n_hifreq > 0)
    {
      /* test if we're within "vibrato" range, and if so update range */
      if (freq * vibmultiple >= x->n_hifreq &&
	  x->n_lofreq * vibmultiple >= freq)
	{
	  if (freq > x->n_hifreq)
	    x->n_hifreq = freq;
	  if (freq < x->n_lofreq)
	    x->n_lofreq = freq;
	}
      else if (x->n_hifreq > 0 && x->n_age > stableperiod)
	{
	  /* if we've been out of range at least 1/2 the
	     last "stableperiod+1" analyses, clear the note */
	  int nbad = 0;
	  for (i = 0, k = x->n_histphase; i < stableperiod + 1; i++)
	    {
	      if (--k < 0)
		k = NHISTPOINT - 1;
	      if (x->n_hist[k].h_freq * vibmultiple <= x->n_hifreq ||
		  x->n_lofreq * vibmultiple <= x->n_hist[k].h_freq)
		nbad++;
	    }
	  if (2 * nbad >= stableperiod + 1)
	    {
	      x->n_hifreq = x->n_lofreq = 0;
	      x->n_age = 0;
	    }
	}
    }
  
  oldhistphase = x->n_histphase - stableperiod;
  if (oldhistphase < 0)
    oldhistphase += NHISTPOINT;
  
  /* look for envelope attacks */
  
  if (x->n_hifreq > 0 && x->n_peaked)
    {
      if (freq > 0 && power > powerthresh &&
	  power > x->n_hist[oldhistphase].h_power *
	  exp((LOG10*0.1)*growththresh))
	{
	  /* clear it and fall through for new stable-note test */
	  x->n_peaked = 0;
	  x->n_hifreq = x->n_lofreq = 0;
	  x->n_age = 0;
	}
    }
  else if (!x->n_peaked)
    {
      if (x->n_hist[oldhistphase].h_power > powerthresh &&
	  x->n_hist[oldhistphase].h_power > power)
	x->n_peaked = 1;
    }
  
  /* test for a new note using a stability criterion. */
  
  if (freq >= 0 &&
      (x->n_hifreq <= 0 || freq > x->n_hifreq || freq < x->n_lofreq))
    {
      t_float testfhi = freq, testflo = freq,
	maxpow = x->n_hist[x->n_histphase].h_freq;
      for (i = 0, k = x->n_histphase; i < stableperiod-1; i++)
	{
	  if (--k < 0)
	    k = NHISTPOINT - 1;
	  if (x->n_hist[k].h_freq > testfhi)
	    testfhi = x->n_hist[k].h_freq;
	  if (x->n_hist[k].h_freq < testflo)
	    testflo = x->n_hist[k].h_freq;
	  if (x->n_hist[k].h_power > maxpow)
	    maxpow = x->n_hist[k].h_power;
	}
#if 0
      if (loud)
	post("freq %.2g testfhi %.2g  testflo %.2g maxpow %.2g",
	     freq, testfhi, testflo, maxpow);
#endif
      if (testflo > 0 && testfhi <= vibmultiple * testflo
	  && maxpow > powerthresh)
	{
	  /* report new note */
	  t_float sumf = 0, sumw = 0, thisw;
	  for (i = 0, k = x->n_histphase; i < stableperiod; i++)
	    {
	      thisw = x->n_hist[k].h_power;
	      sumw += thisw;
	      sumf += thisw*x->n_hist[k].h_freq;
	      if (--k < 0)
		k = NHISTPOINT - 1;
	    }
	  x->n_hifreq = x->n_lofreq = *note = (sumw > 0 ? sumf/sumw : 0);
#if 0
	  /* debugging printout */
	  for (i = 0; i < stableperiod; i++)
	    {
	      int k3 = x->n_histphase - i;
	      if (k3 < 0)
		k3 += NHISTPOINT;
	      startpost("%5.1f ", sigmund_ftom(x->n_hist[k3].h_freq));
	    }
	  post("");
#endif
	  x->n_age = 0;
	  x->n_peaked = 0;
	  return;
	}
    }
  *note = 0;
  return;
}
