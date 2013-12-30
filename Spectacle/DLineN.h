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
#if !defined(__DLineN_h)
#define __DLineN_h

#include "JGFilter.h"

class DLineN : public JGFilter
{
  protected:  
    long inPoint;
    long outPoint;
    long length;
  public:
    DLineN(long max_length);  
    ~DLineN();  
    void clear();
    void setDelay(double lag);
    double tick(double input);
    void putSample(double input);
    double getSample(long lag);
};

#endif

