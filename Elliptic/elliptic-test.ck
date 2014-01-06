Noise n => Elliptic ell => dac;
0.1 => n.gain;
90 => ell.atten;
20 => ell.ripple;
ell.bpf(440,450,610);
minute => now;
