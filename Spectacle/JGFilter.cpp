/* JGFilter Class, by Perry R. Cook, 1995-96
   This is the base class for all filters. To me, most anything is a
   filter, but I'll be a little less general here, and define a filter
   as something which has input(s), output(s), and gain.
*/

#include "JGFilter.h"


JGFilter :: JGFilter(double srate) : _sr(srate)
{
}


JGFilter :: ~JGFilter()
{
}


double JGFilter :: lastOut()
{
   return lastOutput;
}

