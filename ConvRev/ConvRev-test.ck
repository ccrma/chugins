/*
Andrew Zhu Aday M220a Final Project
Summer 2021
ConvRev Chugin Example
*/


SndBuf s => ConvRev cr => dac;

"special:dope" => s.read;

// SndBuf2 ir => blackhole;
SndBuf ir => blackhole;
me.dir() + "./IRs/hagia-sophia.wav" => ir.read;
// me.dir() + "./small.wav" => ir.read; // 44100 sr, stereo */

ir.samples() => cr.order;
for (0 => int i; i < cr.order(); i++) {
    /* cr.coeff(i, ir.valueAt(i*2));  // do this if your IR file is stereo */
    cr.coeff(i, ir.valueAt(i));  // do this if the IR is mono
}

// initialize the conv rev engine, default FFT size is 128 samps
cr.init();

1::eon => now;
