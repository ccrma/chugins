#include <rtdefs.h>
#include "elldefs.h"

class ELL : public Instrument {
   int     nargs, insamps, skip, branch, inchan, nsects;
   float   amp, pctleft, xnorm;
   float   *in, amptabs[2];
   double  *amptable;
   EllSect *es[MAXCHANS];

public:
   ELL();
   virtual ~ELL();
   virtual int init(double p[], int n_args);
   virtual int configure();
   virtual int run();
};

