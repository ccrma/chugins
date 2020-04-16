Phasor phasor => Wrap wrap => blackhole;
phasor.freq(0.5);
phasor.gain(5);

while(true)
{
    <<< phasor.last(), wrap.last() >>>;
    150::ms => now;
}