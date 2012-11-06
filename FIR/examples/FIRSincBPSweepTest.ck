// FIR tests of shifted sinc bandpass filters
// Perry R. Cook, October 2012

Noise n => FIR f => WvOut w => dac;
w.wavFilename("temp.wav");

0.1 => n.gain;

256 => f.order;

0.0 => float center;
while (center < 1.0001)  {
    16.0 => f.sinc;
    f.bpHetero(center);
    0.1 :: second => now;
    center + 0.01 => center;
}

w.closeFile();
