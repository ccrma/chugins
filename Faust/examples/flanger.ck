// name: fin.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
adc => Faust flanger => dac;
// compile faust code
flanger.eval(`
import("effect.lib"); 
process = flanger_demo ;
`);
// parameter dump
flanger.dump();

// time loop
while( true )
{
    // advance time
    100::ms => now;
}
