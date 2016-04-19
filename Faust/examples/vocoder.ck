// name: fin.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
adc => Faust vocoder => dac;
// compile faust code
vocoder.eval(`
import("effect.lib"); 
process = vocoder_demo ;
`);
// parameter dump
vocoder.dump();

// time loop
while( true )
{
    // advance time
    100::ms => now;
}