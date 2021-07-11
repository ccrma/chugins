SubNoise sn => dbRemapOsc erange => SinOsc sin => dac;
5000 => sn.rate; // new random number [-1, 1] every sn.rate samples
2 => sin.sync; // frequency modulation

7 => int nOvertones;
0 => erange.min; // offset from fundamental

[0, 2, 4, 7, 11, 12] @=> int scale[];

while (1)  
{
    Math.random2(0, scale.size()-1) => int inote;
    scale[inote] + 50 => int mnote;
    Math.mtof(mnote) => float fundamental;
    fundamental => sin.freq;
    fundamental * nOvertones => erange.max;
    fundamental => erange.round;
    5::second => now; // time to hold scale note
}
