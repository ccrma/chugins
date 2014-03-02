/*
This is a C hack of the f2c version of the elliptical filter
design program "filter" from the princeton cmix distribution
rossb 17/10/95
*/
#include <math.h>
#ifndef WIN32
#include <unistd.h>
#endif 
#include <stdio.h>
#include <string.h>
#include "setell.h"

#define abs(x) ((x) >= 0 ? (x) : -(x))
#define min(a,b) ((a) <= (b) ? (a) : (b))
#define max(a,b) ((a) >= (b) ? (a) : (b))

typedef struct { double r, i; } complex;
typedef struct { double r, i; } doublecomplex;

static void z_div(doublecomplex *, doublecomplex *, doublecomplex *);
static void d_cnjg(doublecomplex *, doublecomplex *);
static double z_abs(doublecomplex *);
static void z_sqrt(doublecomplex *, doublecomplex *);
static void z_exp(doublecomplex *, doublecomplex *);
void ellips_(double, double, double, double, double, double);
void stuff1_(double q, double r, char *whatsi);
void fresp_(long int, double, double, double, double);
double kay_(double);
void djelf_(double *, double *, double *, double, double);
void setell(double, double, double, double, double, double,double *, int *,short );

static void z_div(doublecomplex *c, doublecomplex *a, doublecomplex *b)
{
  double ratio, den;
  double abr, abi;
  
  if( (abr = b->r) < 0.)
    abr = - abr;
  if( (abi = b->i) < 0.)
    abi = - abi;
  if( abr <= abi )
    {
      if(abi == 0)
	;/*sig_die("complex division by zero", 1);*/
      ratio = b->r / b->i ;
      den = b->i * (1 + ratio*ratio);
      c->r = (a->r*ratio + a->i) / den;
      c->i = (a->i*ratio - a->r) / den;
    }else{
    ratio = b->i / b->r ;
    den = b->r * (1 + ratio*ratio);
    c->r = (a->r + a->i*ratio) / den;
    c->i = (a->i - a->r*ratio) / den;
  }
}

static void d_cnjg(doublecomplex *r, doublecomplex *z)
{
  r->r = z->r;
  r->i = - z->i;
}

static double z_abs(doublecomplex *z)
{
  return (sqrt(z->r*z->r+z->i*z->i));
}

static void z_sqrt(doublecomplex *r, doublecomplex *z)
{
  double mag;
  
  if( (mag = z_abs(z)) == 0.)
    r->r = r->i = 0.;
  else if(z->r > 0)
    {
      r->r = sqrt(0.5 * (mag + z->r) );
      r->i = z->i / r->r / 2;
    }else{
    r->i = sqrt(0.5 * (mag - z->r) );
    if(z->i < 0)
      r->i = - r->i;
    r->r = z->i / r->i / 2;
  }
}

static void z_exp(doublecomplex *r, doublecomplex *z)
{
  double expx;
  
  expx = exp(z->r);
  r->r = expx * cos(z->i);
  r->i = expx * sin(z->i);
}


/* Common Block Declarations */

typedef struct {
  double cn[30], cd[30];
  long int mn, md;
  double const_;
} EllDesignData;

EllDesignData b_1;

struct {
  double k, kprime, cosp0, w1, hpass;
} ellipt_1;

/* set this to print diagnostics */
static short print_diagnostics_ = 0;

void setell(double sr, double f1, double f2, double f3, double ripple, double atten,
	    double *retarr, int *nsects,short print_diagnostics)
{
  double xnyq;
  long m2,jjj,i,i_1;
  
  b_1.mn = 0;
  b_1.md = 0;
  for (i = 0; i < 30; i++)
    {
      b_1.cn[i] = (double)0.;
      b_1.cd[i] = (double)0.;
    }
  
  xnyq = sr / 2.;
  print_diagnostics_=print_diagnostics;
  
  ellips_(f1, f2, f3, ripple, atten, sr);
  
  /* fresp_ calculates   b_1.const_ and prints diagnostic output and frequency response */
  fresp_(200, sr, 0., xnyq, f1);
  
  m2 = b_1.mn / 2;
  *nsects = m2;
  jjj = 0;
  i_1 = b_1.mn;
  for (i = 1; i <= i_1; ++i) {
    retarr[jjj] = b_1.cn[i - 1];
    retarr[jjj + 1] = b_1.cd[i - 1];
    jjj += 2;
  }
  retarr[jjj] = b_1.const_;
}

/*
  ellips_() designs an elliptic filter. all parameters doubles
  
  f3=0 -> lowpass or highpass. f1=passband cutoff. f2=stopband cutoff. 
  f1<f2 -> lowpass. 
  f3>0 -> bandpass. f1,f2 are limits of passband. f3 is limit of 
  either high or low stopband. we require f1<f2. 
  ripple=passband ripple in db. atten=stopband attenuation in db. 
  samr=sampling rate in hz. 
  after gold+rader; written by bilofsky, revised by steiglitz 
  pp.61-65 (elliptic filters), 72,76 (mappings 
  from s-plane to z-plane), 87 (approximation 
  for u0 and evaluation of elliptic functions).
*/
void ellips_(double f1, double f2, double f3, double ripple, double atten, double samr)
{
  /* System generated locals */
  long int i_1;
  double d_1, d_2, d_3, d_4, d_5, d_6;
  
  /* Local variables */
  double a;
  long int i, n;
  double k1;
  long int n2;
  double u0, w2, w3, k1prim, dd, de;
  double kk, pi, nn, tt, kk1;
  double kkp, eps, kk1p;
  
  pi = 3.14159265358979;
  ellipt_1.w1 = pi * 2. * f1 / samr;
  w2 = pi * 2. * f2 / samr;
  w3 = pi * 2. * f3 / samr;
  ellipt_1.hpass = 0.;
  ellipt_1.cosp0 = 0.;
  if (f3 > 0.) {
    goto L1;
  }
  if (f1 < f2) {
    goto L2;
  }
  /*  modify frequencies for high pass. */
  ellipt_1.w1 = pi - ellipt_1.w1;
  w2 = pi - w2;
  ellipt_1.hpass = 1.;
  /*  compute analog frequencies for low/high pass */
 L2:
  ellipt_1.w1 = tan(ellipt_1.w1 * .5);
  w2 = tan(w2 * .5);
  goto L3;
  /*  compute analog frequencies for band pass. */
 L1:
  ellipt_1.cosp0 = cos((ellipt_1.w1 + w2) / 2.) / cos((ellipt_1.w1 - w2) / 
						      2.);
  ellipt_1.w1 = (d_1 = (ellipt_1.cosp0 - cos(ellipt_1.w1)) / sin(
								 ellipt_1.w1), abs(d_1));
  de = w3 - w2;
  if (de < 0.) {
    de = ellipt_1.w1 - w3;
  }
  d_1 = ellipt_1.w1 - de;
  d_3 = w2 + de;
  /* Computing MIN */
  d_5 = (d_2 = (ellipt_1.cosp0 - cos(d_1)) / sin(d_1), abs(d_2)),
    d_6 = (d_4 = (ellipt_1.cosp0 - cos(d_3)) / sin(d_3), abs(d_4));
  w2 = min(d_5,d_6);
  /*  compute params for poles,zeros in lambda plane */
 L3:
  ellipt_1.k = ellipt_1.w1 / w2;
  /* Computing 2nd power */
  d_1 = ellipt_1.k;
  ellipt_1.kprime = sqrt(1. - d_1 * d_1);
  d_1 = ripple * .1;
  eps = sqrt(pow(10., d_1) - 1.);
  d_1 = atten * .05;
  a = pow(10., d_1);
  k1 = eps / sqrt(a * a - 1.);
  /* Computing 2nd power */
  d_1 = k1;
  k1prim = sqrt(1. - d_1 * d_1);
  kk = kay_(ellipt_1.k);
  kk1 = kay_(k1);
  kkp = kay_(ellipt_1.kprime);
  kk1p = kay_(k1prim);
  n = (long int) (kk1p * kk / (kk1 * kkp)) + 1;
  nn = (double) n;
  /* L5: */
  u0 = -kkp * log((sqrt(eps * eps + 1.) + 1.) / eps) / kk1p;
  /*  now compute poles,zeros in lambda plane, */
  /*    transform one by one to z plane. */
  dd = kk / nn;
  tt = kk - dd;
  dd += dd;
  n2 = (n + 1) / 2;
  i_1 = n2;
  for (i = 1; i <= i_1; ++i) {
    if (i << 1 > n) {
      tt = 0.;
    }
    d_1 = -kkp;
    stuff1_(d_1, tt, "zero");
    stuff1_(u0, tt, "pole");
    /* L4: */
    tt -= dd;
  }
} /* ellips_ */


void stuff1_(double q, double r, char *whatsi)
{
  /* System generated locals */
  double d_1, d_2, d_3;
  doublecomplex z_1, z_2, z_3, z_4, z_5, z_6, z_7, z_8;
  
  /* Local variables */
  double cnqp, dnqp, snqp;
  long int j;
  doublecomplex s;
  double omega, x;
  doublecomplex z;
  double sigma;
  double cnr, dnr, snr;
  
  /*    transforms poles and zeros to z-plane; stuffs coeff. array */
  d_1 = ellipt_1.kprime * ellipt_1.kprime;
  djelf_(&snr, &cnr, &dnr, r, d_1);
  d_1 = ellipt_1.k * ellipt_1.k;
  djelf_(&snqp, &cnqp, &dnqp, q, d_1);
  omega = 1 - snqp * snqp * dnr * dnr;
  if (omega == 0.) {
    omega = 1e-30;
  }
  sigma = ellipt_1.w1 * snqp * cnqp * cnr * dnr / omega;
  omega = ellipt_1.w1 * snr * dnqp / omega;
  z_1.r = sigma, z_1.i = omega;
  s.r = z_1.r, s.i = z_1.i;
  j = 1;
  if (ellipt_1.cosp0 == 0.) {
    goto L1;
  }
  j = -1;
 L4:
  d_1 = -ellipt_1.cosp0;
  d_2 = (double) j;
  d_3 = ellipt_1.cosp0 * ellipt_1.cosp0;
  z_7.r = s.r * s.r - s.i * s.i, z_7.i = s.r * s.i + s.i * s.r;
  z_6.r = d_3 + z_7.r, z_6.i = z_7.i;
  z_5.r = z_6.r - 1., z_5.i = z_6.i;
  z_sqrt(&z_4, &z_5);
  z_3.r = d_2 * z_4.r, z_3.i = d_2 * z_4.i;
  z_2.r = d_1 + z_3.r, z_2.i = z_3.i;
  z_8.r = s.r - 1., z_8.i = s.i;
  z_div(&z_1, &z_2, &z_8);
  z.r = z_1.r, z.i = z_1.i;
  goto L3;
 L1:
  z_2.r = s.r + 1., z_2.i = s.i;
  z_3.r = 1. - s.r, z_3.i = -s.i;
  z_div(&z_1, &z_2, &z_3);
  z.r = z_1.r, z.i = z_1.i;
  if (ellipt_1.hpass != 0.) {
    z_1.r = -z.r, z_1.i = -z.i;
    z.r = z_1.r, z.i = z_1.i;
  }
 L3:
  if ((d_1 = z.i, abs(d_1)) <= 1e-9) {
    goto L2;
  }
  if (z.i < 0.) {
    d_cnjg(&z_1, &z);
    z.r = z_1.r, z.i = z_1.i;
  }
  if (strcmp(whatsi, "pole") == 0) {
    goto L5;
  }
  ++b_1.mn;
  b_1.cn[b_1.mn - 1] = z.r * -2.;
  ++b_1.mn;
  /* Computing 2nd power */
  d_1 = z.r;
  /* Computing 2nd power */
  d_2 = z.i;
  b_1.cn[b_1.mn - 1] = d_1 * d_1 + d_2 * d_2;
  goto L6;
 L5:
  ++b_1.md;
  b_1.cd[b_1.md - 1] = z.r * -2.;
  ++b_1.md;
  /* Computing 2nd power */
  d_1 = z.r;
  /* Computing 2nd power */
  d_2 = z.i;
  b_1.cd[b_1.md - 1] = d_1 * d_1 + d_2 * d_2;
 L6:
  if(print_diagnostics_)
    printf("complex %.4s pair at %f +-j %f\n",whatsi,z.r,z.i);
  
  if (j > 0 || r == 0.) {
    return;
  }
  j = 1;
  goto L4;
 L2:
  x = z.r;
  if (strcmp(whatsi, "pole") == 0) {
    goto L7;
  }
  ++b_1.mn;
  b_1.cn[b_1.mn - 1] = -x;
  ++b_1.mn;
  b_1.cn[b_1.mn - 1] = 0.;
  goto L8;
 L7:
  ++b_1.md;
  b_1.cd[b_1.md - 1] = -x;
  ++b_1.md;
  b_1.cd[b_1.md - 1] = 0.;
 L8:
  if(print_diagnostics_)
    printf("real %.4s at %f\n",whatsi,x);
  
  if (j > 0) {
    return;
  }
  j = 1;
  goto L4;
} /* stuff1_ */


void fresp_(long int k, double samr, double f1, double f2, double f3)
{
  /* System generated locals */
  long int i_1, i_2, i_3, i_4, i_5, i_6;
  double d_1;
  doublecomplex z_1, z_2, z_3, z_4, z_5, z_6, z_7, z_8, z_9, z_10;
  
  /* Local variables */
  double freq;
  long int i, j;
  double w, x;
  double y, phase;
  long int m2;
  double db, pi;
  doublecomplex tf, zm, zm2;
  double amp;
  
  /*    plots k pts. of freq. resp. from f1 to f2, norm. at f3 */
  pi = 3.14159265358979;
  m2 = b_1.mn / 2;
  
  if(print_diagnostics_)
    {
      printf("//\nelliptic filter with %ld sections\n",m2);
      i_1 = b_1.mn;
      for (i = 1; i <= i_1; ++i) {
	printf("%f %f ",b_1.cn[i - 1],b_1.cd[i - 1]);
      }
    }
  
  w = pi * f3 / (samr * .5);
  d_1 = w * -1.;
  z_2.r = 0., z_2.i = d_1;
  z_exp(&z_1, &z_2);
  zm.r = z_1.r, zm.i = z_1.i;
  z_1.r = zm.r * zm.r - zm.i * zm.i, z_1.i = zm.r * zm.i + zm.i * zm.r;
  zm2.r = z_1.r, zm2.i = z_1.i;
  tf.r = 1., tf.i = 0.;
  i_1 = b_1.mn;
  for (i = 1; i <= i_1; i += 2) {
    /* L1: */
    i_2 = i - 1;
    z_5.r = b_1.cn[i_2] * zm.r, z_5.i = b_1.cn[i_2] * zm.i;
    z_4.r = z_5.r + 1., z_4.i = z_5.i;
    i_3 = i;
    z_6.r = b_1.cn[i_3] * zm2.r, z_6.i = b_1.cn[i_3] * zm2.i;
    z_3.r = z_4.r + z_6.r, z_3.i = z_4.i + z_6.i;
    z_2.r = tf.r * z_3.r - tf.i * z_3.i, z_2.i = tf.r * z_3.i + tf.i * 
      z_3.r;
    i_4 = i - 1;
    z_9.r = b_1.cd[i_4] * zm.r, z_9.i = b_1.cd[i_4] * zm.i;
    z_8.r = z_9.r + 1., z_8.i = z_9.i;
    i_5 = i;
    z_10.r = b_1.cd[i_5] * zm2.r, z_10.i = b_1.cd[i_5] * zm2.i;
    z_7.r = z_8.r + z_10.r, z_7.i = z_8.i + z_10.i;
    z_div(&z_1, &z_2, &z_7);
    tf.r = z_1.r, tf.i = z_1.i;
  }
  b_1.const_ = 1. / z_abs(&tf);
  
  if(print_diagnostics_)
    printf("\nconst= %f\n",b_1.const_);
  
  i_2 = k;
  for (j = 1; j <= i_2; ++j) {
    freq = f1 + (f2 - f1) * (double) (j - 1) / (double) (k - 1);
    w = pi * freq / (samr * .5);
    d_1 = w * -1.;
    z_2.r = 0., z_2.i = d_1;
    z_exp(&z_1, &z_2);
    zm.r = z_1.r, zm.i = z_1.i;
    z_1.r = zm.r * zm.r - zm.i * zm.i, z_1.i = zm.r * zm.i + zm.i * zm.r;
    zm2.r = z_1.r, zm2.i = z_1.i;
    z_1.r = b_1.const_, z_1.i = 0.;
    tf.r = z_1.r, tf.i = z_1.i;
    i_3 = b_1.mn;
    for (i = 1; i <= i_3; i += 2) {
      /* L2: */
      i_4 = i - 1;
      z_5.r = b_1.cn[i_4] * zm.r, z_5.i = b_1.cn[i_4] * zm.i;
      z_4.r = z_5.r + 1., z_4.i = z_5.i;
      i_5 = i;
      z_6.r = b_1.cn[i_5] * zm2.r, z_6.i = b_1.cn[i_5] * zm2.i;
      z_3.r = z_4.r + z_6.r, z_3.i = z_4.i + z_6.i;
      z_2.r = tf.r * z_3.r - tf.i * z_3.i, z_2.i = tf.r * z_3.i + tf.i *
	z_3.r;
      i_1 = i - 1;
      z_9.r = b_1.cd[i_1] * zm.r, z_9.i = b_1.cd[i_1] * zm.i;
      z_8.r = z_9.r + 1., z_8.i = z_9.i;
      i_6 = i;
      z_10.r = b_1.cd[i_6] * zm2.r, z_10.i = b_1.cd[i_6] * zm2.i;
      z_7.r = z_8.r + z_10.r, z_7.i = z_8.i + z_10.i;
      z_div(&z_1, &z_2, &z_7);
      tf.r = z_1.r, tf.i = z_1.i;
    }
    amp = z_abs(&tf);
    if (amp <= 1e-20) {
      amp = 1e-20;
    }
    x = tf.r;
    y = tf.i;
    phase = 0.;
    if (x == 0. && y == 0.) {
      goto L4;
    }
    phase = 180. / pi * atan2(y, x);
  L4:
    d_1 = max(amp,1e-40);
    db = log10(d_1) * 20.;
    /* L3: */
    if(print_diagnostics_)
      printf("%f \t %f \t %f \t %f\n",freq,phase,amp,db);
  }
  return;
} /* fresp_ */


double kay_(double k)
{
  /* Initialized data */
  static double a[5] = { 1.38629436112,.09666344259,.03590092383,
			 .03742563713,.01451196212 };
  static double b[5] = { .5,.12498593597,.06880248576,.03328355346,
			 .00441787012 };
  
  /* System generated locals */
  double ret_val;
  
  /* Local variables */
  double peta;
  long int i;
  double kk, eta;
  
  /*    computes kay(k)=inverse sn(1) */
  /*    hastings, approx. for dig. comp., p. 172 */
  ret_val = a[0];
  kk = b[0];
  eta = 1. - k * k;
  peta = eta;
  for (i = 2; i <= 5; ++i) {
    ret_val += a[i - 1] * peta;
    kk += b[i - 1] * peta;
    /* L1: */
    peta *= eta;
  }
  ret_val -= kk * log(eta);
  return ret_val;
} /* kay_ */


void djelf_(double *sn, double *cn, double *dn, double x, double sck)
{
  /* System generated locals */
  long int i_1;
  double d_1;
  
  /* Local variables */
  double a, b, c, d = 0.0;
  long int i, k, l;
  double y, cm, geo[12], ari[12];
  
  /*     ssp program: finds jacobian elliptic functions sn,cn,dn. */
  cm = sck;
  y = x;
  if (sck < 0.) {
    goto L3;
  } else if (sck == 0) {
    goto L1;
  } else {
    goto L4;
  }
 L1:
  d = exp(x);
  a = 1. / d;
  b = a + d;
  *cn = 2. / b;
  *dn = *cn;
  a = (d - a) / 2.;
  *sn = a * *cn;
 L2:
  return;
 L3:
  d = 1. - sck;
  cm = -(sck) / d;
  d = sqrt(d);
  y = d * x;
 L4:
  a = 1.;
  *dn = 1.;
  for (i = 1; i <= 12; ++i) {
    l = i;
    ari[i - 1] = a;
    cm = sqrt(cm);
    geo[i - 1] = cm;
    c = (a + cm) * .5;
    if ((d_1 = a - cm, abs(d_1)) - a * 1e-9 <= 0.) {
      goto L7;
    } else {
      goto L5;
    }
  L5:
    cm = a * cm;
    /* L6: */
    a = c;
  }
 L7:
  y = c * y;
  *sn = sin(y);
  *cn = cos(y);
  if (*sn != 0.) {
    goto L8;
  } else {
    goto L13;
  }
 L8:
  a = *cn / *sn;
  c = a * c;
  i_1 = l;
  for (i = 1; i <= i_1; ++i) {
    k = l - i + 1;
    b = ari[k - 1];
    a = c * a;
    c = *dn * c;
    *dn = (geo[k - 1] + a) / (b + a);
    /* L9: */
    a = c / b;
  }
  a = 1. / sqrt(c * c + 1.);
  if (*sn >= 0.) {
    goto L11;
  } else {
    goto L10;
  }
 L10:
  *sn = -a;
  goto L12;
 L11:
  *sn = a;
 L12:
  *cn = c * *sn;
 L13:
  if (sck >= 0.) {
    goto L2;
  } else {
    goto L14;
  }
 L14:
  a = *dn;
  *dn = *cn;
  *cn = a;
  *sn /= d;
  return;
} /* djelf_ */

