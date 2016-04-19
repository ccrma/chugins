// name: fin.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
adc => Faust reverb => dac;
// compile faust code
reverb.eval("import(\"effect.lib\"); process = zita_rev_fdn_demo ;");

// time loop
while( true )
{
    // advance time
    100::ms => now;
}
