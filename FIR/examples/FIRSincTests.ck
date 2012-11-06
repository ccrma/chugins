// FIR tests of Sinc filter(s), Perry R. Cook, 10/12
// Bunch - o - random filter settings

Noise n => FIR f => WvOut w => dac;
w.wavFilename("temp.wav");

0.1 => n.gain;

256 => f.order;

2.0 => f.sinc;
1.0 :: second => now;
f.hpHetero();
1.0 :: second => now;
2.0 => f.sinc;
f.bpHetero(0.5);
1.0 :: second => now;    

8.0 => f.sinc;
1.0 :: second => now;
f.hpHetero();
1.0 :: second => now;
8.0 => f.sinc;
f.bpHetero(0.25);
1.0 :: second => now;    

2.0 => f.sinc;
1.0 :: second => now;
2.5 => f.sinc;
1.0 :: second => now;
3.0 => f.sinc;
1.0 :: second => now;
3.5 => f.sinc;
1.0 :: second => now;
7.333 => f.sinc;
1.0 :: second => now;

16.0 => f.sinc;
1.0 :: second => now;
32.0 => f.sinc;
1.0 :: second => now;
64.0 => f.sinc;
1.0 :: second => now;
128.0 => f.sinc;
1.0 :: second => now;

w.closeFile();
