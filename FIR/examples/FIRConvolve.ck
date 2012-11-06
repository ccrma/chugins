// FIRConvolve.ck  by Perry R. Cook, October 2012
// This uses an FIR filter to store the samples
// of an impulse response (or arbitrary soundfile).
// Not very efficient, (lots of CPU useage) but 
// shows how it works from the definition.  

adc => Gain g => FIR imp => Delay d => dac;
SndBuf s => dac; 
"CCRMAHallShort.wav" => s.read;
g => dac;
0.03 :: second => d.max => d.delay;

s.samples() => imp.order;
for (0 => int i; i < imp.order(); i++) {
    imp.coeff(i,s.last());
    1.0 :: samp => now;
}

10.0 :: second => now;


