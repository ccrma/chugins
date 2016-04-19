// name: fin.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
adc => Faust wah => dac;
// compile faust code
wah.eval(`
import("effect.lib"); 
process = wah4_demo ;
`);
// parameter dump
wah.dump();

// time loop
while( true )
{
    // advance time
    100::ms => now;
}
