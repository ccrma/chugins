#ifndef SETELL_H
#define SETELL_H
/* Design elliptical filter coefficients
   
   f3 = 0-> low or highpass
   f1 = passband cutoff
   f2 = stopband cutoff f1<f2 -> lowpass
   f3>0 -> bandpass. f1,f2 are limits of passband.
   f3 is limit of either high or low stopband. we require f1<f2.
   ripple = passband ripple in db.
   atten = stopband attenuation in db.
   sr = sample rate in Hz
   
   If print_diagnostics is non-zero, program output including frequency
   response data is printed to stderr.
   
   On return, nsects contains the number of sections, retarr contains
   nsects 4 coefficients and the const.
*/
void setell(double sr, double f1, double f2, double f3, double ripple,
            double atten, double *retarr, int *nsects, short print_diagnostics);
#endif /* SETELL_H */

