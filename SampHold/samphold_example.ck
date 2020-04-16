// signal connected to chan(0)
// gets sampled and held every time
// the signal connected to chan(1)
// decreases in amplitude

SampHold SH => blackhole;
SinOsc input => SH.chan(0);
Phasor ctrl => SH.chan(1);

input.freq(210);
ctrl.freq(2.12);


while(true)
{
    <<< ctrl.last(), SH.last() >>>;
    20::ms => now;
}