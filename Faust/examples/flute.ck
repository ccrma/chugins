// name: fau.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
Faust flute => dac;
// evaluate FAUST code
// -- (use ` to avoid escaping " and ')
// -- (auto import libs: music, math, filter, oscillator, effect)
flute.compile("flute.dsp");

flute.dump();

// time loop
while( true )
{
    // set (will auto append /0x00/)
    flute.v("/0x00/Basic_Parameters/gate",1);
    flute.v( "/0x00/Basic_Parameters/freq", Math.random2f(300,1000) );
    // advance time
    300::ms => now;
    flute.v("/0x00/Basic_Parameters/gate",0);
    100::ms => now;
}
