SinOsc sin;
220 => sin.freq;

Patch.help();

Patch p;
Patch s;



p.connect(s);
<<< p.gain >>>;
1::samp => now;