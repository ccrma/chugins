// name: fin.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
adc => Faust reverb => dac;
// compile faust code
reverb.eval(`
process = dm.zita_rev_fdn_demo ;`);
// parameter dump
reverb.dump();

// time loop
while( true )
{
    // advance time
    100::ms => now;
}
