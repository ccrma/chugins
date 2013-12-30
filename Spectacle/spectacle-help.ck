Spectacle spect => dac;
SinOsc boo => dac;
0.1 => boo.gain;

5::second => now;
boo =< dac;
boo => spect;

<<< 0.494 => spect.param >>>;
[4,7] @=> int foo[];
spect.setDelayTimes(foo);
minute => now;
