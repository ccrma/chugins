// name: fin.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
Faust fck => dac;
// compile faust code
fck.compile( "fin.dsp" );

// time loop
while( true )
{
    // set
    fck.v("/0x00/freq", Math.random2f(400,800) );
    // advance time
    100::ms => now;
}
