// name: fau.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
Faust fck => dac;
// evaluate faust code
fck.eval( "import(\"music.lib\"); import(\"oscillator.lib\"); freq=button(\"freq\"); process=sawtooth(freq);" );

// time loop
while( true )
{
    // set
    fck.v("/0x00/freq", Math.random2f(400,800) );
    // advance time
    100::ms => now;
}
