//  FIR Tests of Gaussian Presets, Perry R. Cook, 10/12
//  These are fast, designed to be looked
//  at in a wave and/or spectral display

Impulse imp => FIR f => WvOut w => dac;
w.wavFilename("temp.wav");

1.0 => imp.gain;

2048 => f.order;

(f.order() / 2) :: samp => now;

1.0 => float bandwidth; // actual bandwidth is SRATE/2/this
while (bandwidth < 1024)   {
    bandwidth => f.gaussian;
    1.0 => imp.next;
    2048 :: samp => now;
    2.0 * bandwidth => bandwidth;
}

w.closeFile();
