FIR ChugIn for ChucK/miniAudicle
by Perry R. Cook
Version 1.0
October 2012

Yo!  This here is a ChugIn for ChucK.
It's a general-purpose FIR filter.
You make a new one:

FIR myFilter;

Then specify order:

myFilter.order(N);  // or N => myFilter.order;

which allocates memory for coefficients and delays,
and fills the coefficients with a moving-average
by default (equal gains = 1/N).  NOTE: order here
means the number of coefficients and multiplies,
not strictly the number of delays (as in the standard
definition of filter order).  

You can set the coefficients directly by:

myFilter.coeff(i,x); // i is an integer index, x is a float

or use various built-ins like:

myFilter.sinc(0.125); // this loads it up with a 1/8 band (SR/16) lowpass
                    // Hanning windowed sinc filter.  The higher the
		    // order, the better the stop-band rejection

If you have a low-pass filter in there, you can 
shift it up to make a band-pass:

myFilter.bpHetero(0.5); // this moves the low-pass up to be centered at SR/4

or just complement a low pass filter directly into a highpass:

myFilter.hpHetero();  // this moves it up to SR/2

You can also use the FIR for convolution with a short
soundfile (there's a coupld of examples of that)

Enjoy!!!

PRC

NOTE:  Most of the demos in the examples folder
generate a .wav file so you can inspect it.  If
you don't want that, just comment out the line with:

.wavFilename

in it.  Some of the demos connect the adc to the
dac, so wear headphones or be ready for feedback.








