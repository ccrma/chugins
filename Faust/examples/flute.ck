// instantiate and connect faust => ck
Faust flute => dac;
// evaluate FAUST code
flute.compile("flute.dsp");
// dump parameters
flute.dump();

// time loop
while( true )
{
    // set (will auto append /0x00/)
    flute.v("Basic_Parameters/gate",1);
    flute.v( "Basic_Parameters/freq", Math.random2f(300,1000) );
    // advance time
    300::ms => now;
    flute.v("Basic_Parameters/gate",0);
    100::ms => now;
}
