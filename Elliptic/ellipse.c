#include <stdio.h>
#include "elldefs.h"
#include "setell.h"

#define VERBOSE 0      /* print stuff during filter design (in setell) */

double ellset(double f1, double f2, double f3,
	      double ripple, double atten, double srate);
int get_nsections(void);
int ellpset(EllSect [], float *);
float ellipse(float, int, EllSect [], float);

// ***FIXME: Where did 461 come from?
//    Seems like there only needs to be 4 * MAX_SECTIONS plus 1 for xnorm.
static double coeffs[461];              /* array for coefficients */
static int    nsections = 0;            /* number of sections */


double
ellset(double f1, double f2, double f3,
       double ripple, double atten, double srate)
{
  // ***FIXME: do some input validation here
  
  setell(srate, f1, f2, f3, ripple, atten, coeffs, &nsections, VERBOSE);
  
  //if (nsections < 1 || nsections > MAX_SECTIONS)
  //printf("Ellipse: Filter design failed! Try relaxing specs.\n");
  return 0.0;
}


/* Returns number of filter sections (per channel).
   If zero, means user hasn't called ellset from Minc.
*/
int
get_nsections()
{
  return nsections;
}


int
ellpset(EllSect es[], float *xnorm)
{
  int     i, j;
  EllSect *s;
  
  for (j = i = 0; i < nsections; i++) {
    s = &es[i];
    s->c0 = (float)coeffs[j++];
    s->c1 = (float)coeffs[j++];
    s->c2 = (float)coeffs[j++];
    s->c3 = (float)coeffs[j++];
    s->x1 = s->x2 = s->y1 = s->y2 = 0.0;
  }
  *xnorm = (float)coeffs[j];
  
  return 0;
}


float
ellipse(float sig, int nsects, EllSect es[], float xnorm)
{  
  /* register */ int     i;
  /* register */ float   y0;
  /* register */ EllSect *s;
  
  for (i = 0; i < nsects; i++) {
    s = &es[i];
    
    y0 = sig + s->c0 * s->x1 + s->c2 * s->x2
      - s->c1 * s->y1 - s->c3 * s->y2;
    
    s->x2 = s->x1;
    s->x1 = sig;
    s->y2 = s->y1;
    s->y1 = y0;
    sig = y0;
  }
  return (sig * xnorm);
}
