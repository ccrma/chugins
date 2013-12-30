/* Filter Class, by Perry R. Cook, 1995-96
   This is the base class for all filters. To me, most anything is a
   filter, but I'll be a little less general here, and define a filter
   as something which has input(s), output(s), and gain.
*/
#if !defined(__JGFilter_h)
#define __JGFilter_h

#include "objdefs.h"

class JGFilter
{
  protected:  
    double _sr;
    double gain;
    double *outputs;
    double *inputs;
    double lastOutput;
  public:
    JGFilter(double srate);
    virtual ~JGFilter();
    double lastOut();
};

#endif
