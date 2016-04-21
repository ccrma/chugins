// instantiate and connect faust => ck
adc => Faust cryBaby => dac;
// compile faust code
cryBaby.eval(`process = crybaby_demo ;`);
// parameter dump
cryBaby.dump();

// lfo
SinOsc LFO => blackhole;
1 => LFO.freq;

// time loop
while( true )
{
    // oscillate
    cryBaby.v( "/CRYBABY/Wah_parameter", LFO.last() );
    // advance time
    5::ms => now;
}
