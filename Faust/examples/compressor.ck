// name: fin.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
adc => Faust compressor => dac;
// compile faust code
compressor.eval(`
import("effect.lib"); 
process = compressor_demo;
`);
// parameter dump
compressor.dump();

// time loop
while( true )
{
    // advance time
    100::ms => now;
}
