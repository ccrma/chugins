/* Base class for SPECTACLE* instruments
   John Gibson <johgibso at indiana dot edu>, 11/28/02.
*/

//#define DUMP
//#define DEBUG
#include "SPECTACLE_BASE.h"


/* -------------------------------------------------------- SPECTACLE_BASE -- */
SPECTACLE_BASE :: SPECTACLE_BASE()
{
   first_time = 1;
   iamp_branch = 0;
   oamp_branch = 0;
}


/* ------------------------------------------------------- ~SPECTACLE_BASE -- */
SPECTACLE_BASE :: ~SPECTACLE_BASE()
{
   delete [] input;
   delete [] output;
   delete [] anal_window;
   delete [] synth_window;
   delete [] fft_buf;
   delete [] anal_chans;
   delete [] drybuf;
   delete [] inbuf;
   delete [] outbuf;
   delete dry_delay;
}


/* ---------------------------------------------------------- make_windows -- */
/* Make balanced pair of analysis and synthesis windows.
*/
int SPECTACLE_BASE :: make_windows()
{
   switch (window_type) {
      case Hamming:
         for (int i = 0; i < window_len; i++)
            anal_window[i] = synth_window[i] = 0.54 - 0.46
                                          * cos(TWO_PI * i / (window_len - 1));
         break;
      case Hanning:
         return die(instname(), "Hanning window not implemented.");
         break;
      case Rectangle:
//FIXME: is this right?
         for (int i = 0; i < window_len; i++)
            anal_window[i] = synth_window[i] = 1.0;
         break;
      case Triangle:
         return die(instname(), "Triangle window not implemented.");
         break;
      case Blackman:
         return die(instname(), "Blackman window not implemented.");
         break;
      case Kaiser:
         return die(instname(), "Kaiser window not implemented.");
         break;
   }

   /* When window_len > fft_len, also apply interpolating (sinc) windows to
      ensure that window are 0 at increments of fft_len away from the center
      of the analysis window and of decimation away from the center of the
      synthesis window.
   */
   if (window_len > fft_len) {
      float x;

      /* Take care to create symmetrical windows. */
      x = -(window_len - 1) / 2.;
      for (int i = 0; i < window_len; i++, x += 1.)
         if (x != 0.) {
            anal_window[i] *= fft_len * sin(PI * x / fft_len) / (PI * x);
            if (decimation)
               synth_window[i] *= decimation * sin(PI * x / decimation)
                                                               / (PI * x);
         }
   }

   /* Normalize windows for unity gain across unmodified
      analysis-synthesis procedure.
   */
   float sum = 0.0;
   for (int i = 0; i < window_len; i++)
      sum += anal_window[i];

   for (int i = 0; i < window_len; i++) {
      float afac = 2. / sum;
      float sfac = window_len > fft_len ? 1. / afac : afac;
      anal_window[i] *= afac;
      synth_window[i] *= sfac;
   }

   if (window_len <= fft_len && decimation) {
      sum = 0.0;
      for (int i = 0; i < window_len; i += decimation)
         sum += synth_window[i] * synth_window[i];
      sum = 1.0 / sum;
      for (int i = 0; i < window_len; i++)
         synth_window[i] *= sum;
   }

   return 0;
}


/* ---------------------------------------------------- resample_functable -- */
/* Accepts a function table of <oldsize> elements, describing a series of
   line segments. Resamples this table using linear interpolation, so that
   its shape is described by <newsize> elements. Returns a new table,
   allocated with new, containing this resampled data.  Caller is responsible
   for deleting this table.
*/
double * SPECTACLE_BASE :: resample_functable(double *table, int oldsize,
                                                               int newsize)
{
   double *newtable = new double [newsize];

   if (newsize == oldsize) {                  // straight copy
      for (int i = 0; i < newsize; i++)
         newtable[i] = table[i];
   }
   else {
      double incr = (double) oldsize / (double) newsize;
      double f = 0.0;
      for (int i = 0; i < newsize; i++) {
         int n = (int) f;
         double frac = f - (double) n;
         double diff = 0.0;
         if (frac) {
            double next = (n + 1 < oldsize) ? table[n + 1] : table[oldsize - 1];
            diff = next - table[n];
         }
         newtable[i] = table[n] + (diff * frac);
         f += incr;
      }
   }
   return newtable;
}


/* ------------------------------------------------------------------ init -- */
WindowType SPECTACLE_BASE :: getWindowType(double pval)
{
   int intval = int(pval);
   WindowType type = Hamming;

   switch (intval) {
      case 0:
         type = Hamming;
         break;
      case 1:
         type = Hanning;
         break;
      case 2:
         type = Rectangle;
         break;
      case 3:
         type = Triangle;
         break;
      case 4:
         type = Blackman;
         break;
      case 5:
         type = Kaiser;
         break;
      default:
         die(instname(), "Invalid window type %d\n.", intval);
         break;
   }
   return type;
}

int SPECTACLE_BASE :: init(double p[], int n_args)
{
   float outskip = p[0];
   float inskip = p[1];
   inputdur = p[2];
   amp = p[3];
   ringdur = p[4];
   fft_len = (int) p[5];
   window_len = (int) p[6];
   window_type = getWindowType(p[7]);
   float overlap = p[8];

   /* Make sure FFT length is a power of 2 <= MAXFFTLEN and <= RTBUFSAMPS. */
   bool valid = false;
   for (int x = 1; x <= MAXFFTLEN; x *= 2) {
      if (fft_len == x) {
         valid = true;
         break;
      }
   }
   if (!valid || fft_len > MAXFFTLEN)
      return die(instname(), "FFT length must be a power of two <= %d",
                                                                  MAXFFTLEN);

// FIXME: now this isn't a problem; instead, decimation can't be larger
// than RTBUFSAMPS.  But must couch errmsg in terms of overlap and fft length,
// not decimation...
#if 0
   if (fft_len > RTBUFSAMPS)
      return die(instname(),
                 "FFT length must be a power of two less than or equal\n"
                 "to the output buffer size set in rtsetparams (currently %d).",
                 RTBUFSAMPS);
#endif
   half_fft_len = fft_len / 2;
   fund_anal_freq = SR / (float) fft_len;

   /* Make sure window length is a power of 2 >= FFT length. */
   valid = false;
   for (int x = fft_len; x <= MAXWINDOWLEN; x *= 2) {
      if (window_len == x) {
         valid = true;
         break;
      }
   }
   if (!valid)
      return die(instname(),
                     "Window length must be a power of two >= FFT length\n"
                     "(currently %d) and <= %d.", fft_len, MAXWINDOWLEN);

   /* Make sure overlap is a power of 2 in our safety range. */
   valid = false;
//FIXME: need to adjust MINOVERLAP so that iterations is never 0 in run()
// This might depend upon window_len??
   for (float x = MINOVERLAP; x <= MAXOVERLAP; x *= 2.0) {
      if (overlap == x) {
         valid = true;
         break;
      }
   }
   if (!valid)
      return die(instname(),
                 "Overlap must be a power of two between %g and %g.",
                 MINOVERLAP, MAXOVERLAP);
   int_overlap = (int) overlap;

   /* derive decimation from overlap */
   decimation = (int) (fft_len / overlap);

   DPRINT2("fft_len=%d, decimation=%d\n", fft_len, decimation);

   if (pre_init(p, n_args) != 0)    /* can modify ringdur */
      return DONT_SCHEDULE;

   iamparray = floc(1);
   if (iamparray) {
      int lenamp = fsize(1);
      tableset(SR, inputdur, lenamp, iamptabs);
   }
   else
      rtcmix_advise(instname(), "Setting input amplitude curve to all 1's.");

   oamparray = floc(2);
   if (oamparray) {
      int lenamp = fsize(2);
      tableset(SR, inputdur + ringdur, lenamp, oamptabs);
   }
   else
      rtcmix_advise(instname(), "Setting output amplitude curve to all 1's.");

   if (rtsetinput(inskip, this) == -1)
      return DONT_SCHEDULE;
   if (inchan >= inputChannels())
      return die(instname(), "You asked for channel %d of a %d-channel file.",
                                                   inchan, inputChannels());

   /* <latency> is the delay before the FFT looks at actual input rather than
      zero-padding.  Need to let inst run long enough to compensate for this.
   */
   window_len_minus_decimation = window_len - decimation;
   latency = window_len + window_len_minus_decimation;
   float latency_dur = latency * (1.0 / SR);
   if (rtsetoutput(outskip, latency_dur + inputdur + ringdur, this) == -1)
      return DONT_SCHEDULE;
   total_insamps = (int)(inputdur * SR);    /* without latency_dur */
   input_end_frame = total_insamps + latency;
   DPRINT1("input_end_frame=%d\n", input_end_frame);

   input = new float [window_len];           /* input buffer */
   output = new float [window_len];          /* output buffer */
   anal_window = new float [window_len];     /* analysis window */
   synth_window = new float [window_len];    /* synthesis window */
   fft_buf = new float [fft_len];            /* FFT buffer */
   anal_chans = new float [fft_len + 2];     /* analysis channels */

   if (make_windows() != 0)
      return DONT_SCHEDULE;

   /* Delay dry output by window_len - decimation to sync with wet sig. */
   drybuf = new float [decimation];
   dry_delay = new DLineN(window_len);
   dry_delay->setDelay((float) window_len_minus_decimation);

   /* Init iamp and oamp to starting amplitudes. */
   iamp = (iamparray == NULL) ? 1.0 : iamparray[0];
   oamp = (oamparray == NULL) ? amp : oamparray[0];

   skip = (int) (SR / (float) resetval);

   if (post_init(p, n_args) != 0)
      return DONT_SCHEDULE;

   return nSamps();
}


/* ---------------------------------------------------------------- shiftin -- */
void SPECTACLE_BASE :: shiftin()
{
   DPRINT1("shiftin (cursamp=%d)\n", currentFrame());

   int _cursamp = currentFrame();

   /* Shift samples in <input> from right to left by <decimation>. */
   for (int i = 0; i < window_len_minus_decimation; i++)
      input[i] = input[i + decimation];

   /* Copy <decimation> samples from <inbuf> to right end of <input> and
      to left end of <drybuf>.
   */
   int j = 0;
   for (int i = window_len_minus_decimation; i < window_len; i++, j++) {
      if (--iamp_branch <= 0) {
         /* Since inbuf_readptr trails inbuf_writeptr, we delay the start of
            the envelope until we reach actual input rather than zero padding.
         */
         if (iamparray && _cursamp >= window_len)
            iamp = tablei(_cursamp - window_len, iamparray, iamptabs);
         iamp_branch = skip;
      }
      float sig = inbuf_readptr[inchan] * iamp;
      drybuf[j] = dry_delay->tick(sig);
      input[i] = sig;
      inbuf_readptr += inputChannels();
      if (inbuf_readptr >= inbuf_endptr) {
         inbuf_readptr = inbuf_startptr;
         /* <inbuf_startptr> might not == <inbuf> in first call to run(). */
         inbuf_startptr = inbuf;
         DPRINT1("shiftin: resetting inbuf_readptr=%p\n", inbuf_readptr);
      }
      _cursamp++;
   }
}


/* ------------------------------------------------------------------ fold -- */
/* Multiply input array by the analysis window, both of length <window_len>.
   Using modulus arithmetic, fold and rotate windowed input into FFT buffer
   of length <fft_len>, according to current input time <n>.
*/
void SPECTACLE_BASE :: fold(int n)
{
   for (int i = 0; i < fft_len; i++)
      fft_buf[i] = 0.0;

   n %= fft_len;

   for (int i = 0; i < window_len; i++) {
      fft_buf[n] += input[i] * anal_window[i];
      if (++n == fft_len)
         n = 0;
   }
}


/* ----------------------------------------------------------- leanconvert -- */
/* <fft_buf> is a spectrum in JGrfft format, i.e. it contains <half_fft_len> * 2
   real values, arranged in pairs of real and imaginary values, except for
   the first two values, which are the real parts of 0 and Nyquist frequencies.
   Converts these into <half_fft_len> + 1 pairs of magnitude and phase values,
   and stores them into the output array <anal_chans>.
*/
void SPECTACLE_BASE :: leanconvert()
{
   int   real_index, imag_index, amp_index, phase_index;
   float a, b;

   for (int i = 0; i <= half_fft_len; i++) {
      real_index = amp_index = i << 1;
      imag_index = phase_index = real_index + 1;
      if (i == half_fft_len) {
         a = fft_buf[1];
         b = 0.0;
      }
      else {
         a = fft_buf[real_index];
         b = (i == 0) ? 0.0 : fft_buf[imag_index];
      }
      anal_chans[amp_index] = hypot(a, b);
      anal_chans[phase_index] = -atan2(b, a);
   }
}


/* ------------------------------------------------------- flush_dry_delay -- */
void SPECTACLE_BASE :: flush_dry_delay()
{
   if (currentFrame() < input_end_frame + window_len_minus_decimation) {
      for (int i = 0; i < decimation; i++)
         drybuf[i] = dry_delay->tick(0.0);
   }
}


/* --------------------------------------------------------- leanunconvert -- */
/* leanunconvert essentially undoes what leanconvert does, i.e., it turns
   <half_fft_len> + 1 pairs of amplitude and phase values in <anal_chans>
   into <half_fft_len> pairs of complex spectrum data (in JGrfft format) in
   output array <fft_buf>.
*/
void SPECTACLE_BASE :: leanunconvert()
{
   int   real_index, imag_index, amp_index, phase_index;
   float mag, phase;

   for (int i = 0; i <= half_fft_len; i++) {
      real_index = amp_index = i << 1;
      imag_index = phase_index = real_index + 1;
      if (i == half_fft_len)
         real_index = 1;
      mag = anal_chans[amp_index];
      phase = anal_chans[phase_index];
      fft_buf[real_index] = mag * cos(phase);
      if (i != half_fft_len)
         fft_buf[imag_index] = -mag * sin(phase);
   }
}


/* ------------------------------------------------------------ overlapadd -- */
/* <fft_buf> is a folded spectrum of length <fft_len>.  <output> and
   <synth_window> are of length <window_len>.  Overlap-add windowed,
   unrotated, unfolded <fft_buf> data into <output>.
*/
void SPECTACLE_BASE :: overlapadd(int n)
{
   n %= fft_len;
   for (int i = 0; i < window_len; i++) {
      output[i] += fft_buf[n] * synth_window[i];
      if (++n == fft_len)
         n = 0;
   }
}


/* -------------------------------------------------------------- shiftout -- */
void SPECTACLE_BASE :: shiftout()
{
   int _cursamp = currentFrame();

   for (int i = 0; i < decimation; i++) {
      if (--oamp_branch <= 0) {
         if (oamparray && (_cursamp >= latency))
            oamp = tablei(_cursamp - latency, oamparray, oamptabs) * amp;
         oamp_branch = skip;
      }
      float sig;
      if (_cursamp < input_end_frame)
         sig = ((output[i] * wetdry) + ((1.0 - wetdry) * drybuf[i])) * oamp;
      else
         sig = output[i] * wetdry * oamp;
      if (outputChannels() == 2) {
         *outbuf_writeptr++ = sig * pctleft;
         *outbuf_writeptr++ = sig * (1.0 - pctleft);
      }
      else
         *outbuf_writeptr++ = sig;
      if (outbuf_writeptr >= outbuf_endptr) {
         outbuf_writeptr = outbuf;
         DPRINT1("shiftout: resetting outbuf_writeptr=%p\n", outbuf_writeptr);
      }
      _cursamp++;
   }

   /* shift samples in <output> from right to left by <decimation> */
   for (int i = 0; i < window_len_minus_decimation; i++)
      output[i] = output[i + decimation];
   for (int i = window_len_minus_decimation; i < window_len; i++)  
      output[i] = 0.0;
}


/* ------------------------------------------------------------------- run -- */
int SPECTACLE_BASE :: run()
{
   if (first_time) {
      /* create segmented input buffer */
      int num_segments = (window_len / RTBUFSAMPS) + 2;
      int extrasamps = RTBUFSAMPS - framesToRun();
      if (extrasamps)
         num_segments++;
      int inbuf_samps = num_segments * RTBUFSAMPS * inputChannels();
      inbuf = new float [inbuf_samps];
      for (int i = 0; i < inbuf_samps; i++)
         inbuf[i] = 0.0;

      /* Read ptr chases write ptr by <window_len>.  Set read ptr to a position
         <window_len> frames from right end of input buffer.  Set write ptr to
         beginning of buffer, or, if framesToRun() is not the same as the RTcmix
         buffer size, set it so that the next run invocation after this one
         will find the write ptr at the start of the second buffer segment.
      */
      inbuf_readptr = inbuf + (inbuf_samps - (window_len * inputChannels()));
      inbuf_startptr = inbuf + (extrasamps * inputChannels());
      inbuf_writeptr = inbuf_startptr;
      inbuf_endptr = inbuf + inbuf_samps;

      int outbuf_samps = num_segments * RTBUFSAMPS * outputChannels();
      outbuf = new float [outbuf_samps];
      for (int i = 0; i < outbuf_samps; i++)
         outbuf[i] = 0.0;

      outbuf_readptr = outbuf + (outbuf_samps
                                       - (framesToRun() * outputChannels()));
      outbuf_writeptr = outbuf_readptr;
      outbuf_endptr = outbuf + outbuf_samps;
      first_time = 0;

      DPRINT3("framesToRun()=%d, extrasamps=%d, num_segments=%d\n",
                                 framesToRun(), extrasamps, num_segments);
      DPRINT3("inbuf_samps=%d, inbuf_readptr=%p, inbuf_writeptr=%p\n",
                                 inbuf_samps, inbuf_readptr, inbuf_writeptr);
      DPRINT3("outbuf_samps=%d, outbuf_readptr=%p, outbuf_writeptr=%p\n",
                                 outbuf_samps, outbuf_readptr, outbuf_writeptr);
   }

   const int insamps = framesToRun() * inputChannels();
   if (currentFrame() < total_insamps)
      rtgetin(inbuf_writeptr, this, insamps);

   int iterations = framesToRun() / decimation;
   if (framesToRun() < RTBUFSAMPS)
      iterations++;

   DPRINT1("iterations=%d\n", iterations);

   for (int i = 0; i < iterations; i++) {
      if (currentFrame() < input_end_frame) {
         DPRINT1("taking input...cursamp=%d\n", currentFrame());
         shiftin();
         fold(currentFrame());
         JGrfft(fft_buf, half_fft_len, FORWARD);
         leanconvert();
      }
      else
         flush_dry_delay();
      modify_analysis();
      leanunconvert();
      JGrfft(fft_buf, half_fft_len, INVERSE);
      overlapadd(currentFrame());
      shiftout();

      increment(decimation);
   }

   if (currentFrame() < input_end_frame) {
      inbuf_writeptr += insamps;
      if (inbuf_writeptr >= inbuf_endptr)
         inbuf_writeptr = inbuf;
   }

   rtbaddout(outbuf_readptr, framesToRun());
   outbuf_readptr += framesToRun() * outputChannels();
   if (outbuf_readptr >= outbuf_endptr)
      outbuf_readptr = outbuf;

   return framesToRun();
}


