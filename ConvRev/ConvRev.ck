//-----------------------------------------------------------------------------
// name: ConvRev.ck
// desc: Example convolving a 16s Hagia Sophia IR with special:dope
//
// author: Andrew Zhu Aday (https://ccrma.stanford.edu/~azaday/)
// date: Fall 2023
//-----------------------------------------------------------------------------

// Set up signal processing chain =============================================

SndBuf buf => ConvRev cr => dac;  // wet
buf => Gain dryGain => dac;       // dry

SndBuf ir => blackhole;           // impulse response

dryGain.gain(0.4);                // set dry gain to 40%

me.dir() + "./IRs/hagia-sophia.wav" => ir.read;   // load IR file
"special:dope" => buf.read;                       // load "dope" sample

// Initialize Convolution Engine ==============================================

// Set the order of the convolution filter
ir.samples() => cr.order;  

// copy over the IR samples
for (0 => int i; i < cr.order(); i++)
    cr.coeff(i, ir.valueAt(i));  

// initialize the conv rev engine with a default FFT size of 128 samples
cr.init();

// Loop =======================================================================

while (true) {
    (cr.order() + buf.samples())::samp => now;  // wait for reverb to finish
    0 => buf.pos;  // restart sample
}