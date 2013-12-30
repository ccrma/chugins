/* Non-Interpolating Delay Line Object by Perry R. Cook 1995-96
   This one uses a delay line of maximum length specified on creation.
   A non-interpolating delay line should be used in non-time varying
   (reverb) or non-critical (????) applications.

   JGG added alternative API, to be used in place of setDelay / tick.
   Useful mainly for situations where you want multiple taps, which you
   can't do with the other API. To summarize the two API's:

    (1) setDelay / tick:
          setDelay(lag);
          output = tick(input);     // output is <lag> samps older than input

    (2) putSample / getSample:
          putSample(input);         // like tick, but no output returned
          output = getSample(lag);  // output is lag samps older than last input
          out2 = getSample(lag2);   // can call again before next putSample

   Note that in the interest of efficiency, putSample does not maintain the
   correct value for outPoint. (If you want to use the first API after having
   used the second one for a DLineN object, call setDelay to init outPoint.)
*/
#include "DLineN.h"


DLineN :: DLineN(long max_length) : JGFilter(0)
{
   /* Add 2 to let user call setDelay(max_length) next without worrying
      about outPoint stepping on inPoint.  -JGG
   */
   length = max_length + 2;       

   inputs = new double [length];
   outPoint = 0;
   inPoint = length >> 1;                  // by default, .5 of max delay  -JGG
   this->clear();
   outputs = NULL;                         // unused
}


DLineN :: ~DLineN()
{
   delete [] inputs;
}


void DLineN :: clear()
{
   for (long i = 0; i < length; i++)
      inputs[i] = 0.0;
   lastOutput = 0.0;
}


void DLineN :: setDelay(double lag)        // in samples! ignores fract. part
{
// assert((long)lag < length);

   outPoint = inPoint - (long) lag;          // read chases write
   while (outPoint < 0)
      outPoint += length;                    // modulo maximum length
}


/* Puts <sample> into delay line, returns oldest sample in delay line.
*/
double DLineN :: tick(double input)           // take one, yield one
{                                              
   inputs[inPoint++] = input;
   if (inPoint == length)                     // check for end condition
      inPoint -= length;
   lastOutput = inputs[outPoint++];
   if (outPoint >= length)
      outPoint -= length;
   return lastOutput;
}


/* Like tick, but doesn't bother maintaining outPoint. Use this in
   conjunction with getSample only!
*/
void DLineN :: putSample(double input)
{
   inputs[inPoint++] = input;                    // input next sample
   if (inPoint == length)                        // check for end condition
      inPoint -= length;
}


/* Get sample from delay line that is <lag> samples behind the most recent
   sample to enter the delay line. If <lag> is longer than length of delay
   line, it wraps around. (So check first.)
   (Note: this is like the cmix genlib delget, except it takes the number
   of samples of delay rather than a delay time in seconds.)
*/
double DLineN :: getSample(long lag)
{
   long outpt = inPoint - lag;
   while (outpt < 0)
      outpt += length;
   lastOutput = inputs[outpt];
   return lastOutput;
}


