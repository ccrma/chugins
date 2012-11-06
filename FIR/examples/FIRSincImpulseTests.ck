// FIR Sinc filter tests, Perry R. Cook, 10/12
// Stream of rapid impulse responses, designed
// be looked at in wave/spectral display

Impulse imp => FIR f => WvOut w => dac;
w.wavFilename("temp.wav");

1.0 => imp.gain;

4096 => f.order;

4096 :: samp => now;
4096 :: samp => now;

2.0 => f.sinc; 1.0 => imp.next;
4096 :: samp => now;
f.hpHetero(); 1.0 => imp.next;
4096 :: samp => now;
2.0 => f.sinc; 1.0 => imp.next;
f.bpHetero(0.5);
4096 :: samp => now;    

8.0 => f.sinc; 1.0 => imp.next;
4096 :: samp => now;
f.hpHetero(); 1.0 => imp.next;
4096 :: samp => now;
8.0 => f.sinc; 1.0 => imp.next;
f.bpHetero(0.5);
4096 :: samp => now;    

2.0 => f.sinc; 1.0 => imp.next;
4096 :: samp => now;
2.5 => f.sinc; 1.0 => imp.next;
4096 :: samp => now;
3.0 => f.sinc; 1.0 => imp.next;
4096 :: samp => now;
3.5 => f.sinc; 1.0 => imp.next;
4096 :: samp => now;
7.333 => f.sinc; 1.0 => imp.next;
4096 :: samp => now;

8.0 => f.sinc; 1.0 => imp.next;
4096 :: samp => now;
16.0 => f.sinc; 1.0 => imp.next;
4096 :: samp => now;
32.0 => f.sinc; 1.0 => imp.next;
4096 :: samp => now;
64.0 => f.sinc; 1.0 => imp.next;
4096 :: samp => now;
128.0 => f.sinc; 1.0 => imp.next;
4096 :: samp => now;

w.closeFile();
