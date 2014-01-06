
Noise n => Elliptic ell => dac;
0.1 => n.gain;

80 => ell.atten;
10 => ell.ripple;
ell.bpf(500,600,650);
minute => now;
