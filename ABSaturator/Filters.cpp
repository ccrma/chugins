
#include "Filters.h"
#include "chugin.h"
#include <math.h>

//------------------------------------------------------------------------------
void Filters::bilinearTranform(double acoefs[], double dcoefs[], double fs)
{
	double b0, b1, b2, a0, a1, a2;		//storage for continuous-time filter coefs
	double bz0, bz1, bz2, az0, az1, az2;	// coefs for discrete-time filter.
	
	// For easier looking code...unpack
	b0 = acoefs[0]; b1 = acoefs[1]; b2 = acoefs[2]; 
    a0 = acoefs[3]; a1 = acoefs[4]; a2 = acoefs[5];
	
	
	// TODO: apply bilinear transform
	///////////////START//////////////////
	bz0 = 1.0; bz1 = 0.0; bz2 = 0.0; 
    az0 = 1.0; az1 = 0.0; az2 = 0.0;
	
	az0 = a2*4*fs*fs + a1*2*fs + a0;
    
    bz2 = (b2*4*fs*fs - b1*2*fs + b0) / az0;
    bz1 = (-b2*8*fs*fs + 2*b0) / az0;
    bz0 = (b2*4*fs*fs+ b1*2*fs + b0) / az0;
    az2 = (a2*4*fs*fs - a1*2*fs + a0) / az0;
    az1 = (-a2*8*fs*fs + 2*a0) / az0;
    
	
	////////////////END/////////////////////
	
	// return coefficients to the output
	dcoefs[0] = bz0; dcoefs[1] = bz1; dcoefs[2] = bz2; 
    dcoefs[3] = az1; dcoefs[4] = az2;
    
}


//------------------------------------------------------------------------------
void Filters::designParametric(double* dcoefs, double center, double gain, double qval, double fs)
// design parametric filter based on input center frequency, gain, Q and sampling rate
{
	double b0, b1, b2, a0, a1, a2;		//storage for continuous-time filter coefs
	double acoefs[6];
    
	//Design parametric filter here. Filter should be of the form
	//
	//    2
	// b2s  + b1s + b0
	// ---------------
	//    2
	// a2s  + a1s + a0
	//
	// Parameters are center frequency in Hz, gain in dB, and Q.
	
	
	//TODO: design analog filter based on input gain, center frequency and Q	
	b0 = 1.0; b1 = 0.0; b2 = 0.0; 
    a0 = 1.0; a1 = 0.0; a2 = 0.0;
 	///////////////START//////////////////
	
    double Q = qval;
    double wc = center*2*CK_ONE_PI;
    
	// 2nd order butterworth RLP
	b2 = 0.0;
    b1 = 0.0;
    b0 = 1.0;
    a2 = 1.0/(wc*wc);
    a1 = 1.0/(Q*wc);
    a0 = 1.0;
	
	////////////////END/////////////////////	
	// pack the analog coeffs into an array and apply the bilinear tranform
	acoefs[0] = b0; acoefs[1] = b1; acoefs[2] = b2; 
    acoefs[3] = a0; acoefs[4] = a1; acoefs[5] = a2;
	
	// inputs the 6 analog coeffs, output the 5 digital coeffs
	bilinearTranform(acoefs, dcoefs, fs);
	
}

