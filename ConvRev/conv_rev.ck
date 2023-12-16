/*
Andrew Zhu Aday M220a Final Project
ConvRev Chugin Examples

SndBuf => ConvRev
*/

ConvRev cr; Gain g;
1 => g.gain;  // change me!

cr => g => dac;

SndBuf2 ir => blackhole;
/* "../IRs/church_of_gesu.wav" => ir.read; // 44100 sr, mono */
me.dir() + "./IRs/hagia_sophia.wav" => ir.read; // 48000 sr, mono
/* "../IRs/musikvereinIR.wav" => ir.read; // 48000 sr, mono */
/* "../IRs/small.wav" => ir.read; // 44100 sr, stereo */

ir.samples() => cr.order;
cr.order() => int order;
for (0 => int i; i < order; i++) {
    /* cr.coeff(i, ir.valueAt(i*2));  // do this if your IR file is stereo */
    cr.coeff(i, ir.valueAt(i));  // do this if the IR is mono
}
cr.init();  // initialize the conv rev engine, default FFT size is 128 samps


SndBuf s;
"denon9Mono48.wav" => s.read;

s => cr;

// record
dac => WvOut2 w => blackhole;			// write out .wav file
w.wavFilename("hs-convolve.wav");
/* w.wavFilename("gesu-convolve.wav"); */
/* w.wavFilename("musikverein-convolve.wav"); */
/* w.wavFilename("musikverein-convolve-2.wav"); */


s.samples()::samp + order::samp + 2 ::second => now;
w.closeFile();
