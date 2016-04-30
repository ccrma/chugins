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
<<<<<<< HEAD
    200::ms => now;
    flute.v("/0x00/Basic_Parameters/gate",0);
    50::ms => now;
=======
    300::ms => now;
    flute.v("Basic_Parameters/gate",0);
    100::ms => now;
>>>>>>> 90e91e52cfcc063ce85b55ec7755b1f3bd652a36
}
