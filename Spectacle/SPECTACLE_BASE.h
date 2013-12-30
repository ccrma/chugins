#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <ugens.h>
//#include <mixerr.h>
//#include <Instrument.h>
//#include <rt.h>
//#include <rtdefs.h>
//#include <assert.h>
//#include <objlib.h>
#include "fft.h"

#define MAXFFTLEN    4096
#define MAXWINDOWLEN MAXFFTLEN * 8
#define MINOVERLAP   0.25
#define MAXOVERLAP   64.0
#define MAXDELTIME   20.0

#ifdef DEBUG
   #define DPRINT(msg)                    printf((msg))
   #define DPRINT1(msg, arg)              printf((msg), (arg))
   #define DPRINT2(msg, arg1, arg2)       printf((msg), (arg1), (arg2))
   #define DPRINT3(msg, arg1, arg2, arg3) printf((msg), (arg1), (arg2), (arg3))
#else
   #define DPRINT(msg)
   #define DPRINT1(msg, arg)
   #define DPRINT2(msg, arg1, arg2)
   #define DPRINT3(msg, arg1, arg2, arg3)
#endif

typedef enum {
   Hamming = 0,
   Hanning,
   Rectangle,
   Triangle,
   Blackman,
   Kaiser
} WindowType;

class SPECTACLE_BASE {

private:
   int      iamp_branch, oamp_branch;
   float    amp, iamp, oamp;
   float    *anal_window, *synth_window, *input, *output, *fft_buf, *drybuf;
   float    *inbuf, *inbuf_startptr, *inbuf_readptr, *inbuf_writeptr,
            *inbuf_endptr;
   float    *outbuf, *outbuf_startptr, *outbuf_readptr, *outbuf_writeptr,
            *outbuf_endptr;
   float    iamptabs[2], oamptabs[2];
   double   *iamparray, *oamparray;
   DLineN   *dry_delay;
   WindowType window_type;
protected:
   int      inchan, skip, total_insamps, first_time, int_overlap;
   int      fft_len, window_len, decimation, window_len_minus_decimation,
            half_fft_len, latency, input_end_frame;
   float    wetdry, pctleft, inputdur, ringdur, fund_anal_freq;
   float    *anal_chans;

public:
   SPECTACLE_BASE();
   virtual ~SPECTACLE_BASE();
   virtual int init(double p[], int n_args);
   virtual int run();
protected:
   double *resample_functable(double *, int, int);
   int make_windows();
   void shiftin();
   void fold(int);
   void leanconvert();
   void flush_dry_delay();
   void leanunconvert();
   void overlapadd(int);
   void shiftout();
   virtual int pre_init(double p[], int n_args) = 0;
   virtual int post_init(double p[], int n_args) = 0;
   virtual void modify_analysis() = 0;
   virtual const char *instname() = 0;
private:
   WindowType getWindowType(double pval);
};

inline int odd(long x) { return (x & 0x00000001); }

