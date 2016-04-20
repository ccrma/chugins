// name: fau.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
Faust clarinet => dac;
// evaluate FAUST code
// -- (use ` to avoid escaping " and ')
// -- (auto import libs: music, math, filter, oscillator, effect)
clarinet.compile("clarinet.dsp");

clarinet.dump();

// time loop
while( true )
{
    // set (will auto append /0x00/)
    clarinet.v("/0x00/Basic_Parameters/gate",1);
    clarinet.v( "/0x00/Basic_Parameters/freq", Math.random2f(100,800) );
    // advance time
    300::ms => now;
    clarinet.v("/0x00/Basic_Parameters/gate",0);
    100::ms => now;
}
