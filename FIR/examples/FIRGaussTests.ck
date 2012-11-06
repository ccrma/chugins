//  FIR Gaussian Preset and Transformation Tests
//  Perry R. Cook, 10/12

Noise n => FIR f => WvOut w => dac;
w.wavFilename("temp.wav");

0.1 => n.gain;

128 => f.order;

2.0 => f.gaussian;
1.0 :: second => now;
4.0 => f.gaussian;
1.0 :: second => now;
8.0 => f.gaussian;
1.0 :: second => now;
16.0 => f.gaussian;
1.0 :: second => now;
32.0 => f.gaussian;
1.0 :: second => now;
64.0 => f.gaussian;
1.0 :: second => now;
128.0 => f.gaussian;
1.0 :: second => now;

w.closeFile();
