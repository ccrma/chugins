// name: fin.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
adc => Faust cryBaby => dac;
// compile faust code
cryBaby.eval(`
import("effect.lib"); 
process = crybaby_demo ;
`);
// parameter dump
cryBaby.dump();

// time loop
while( true )
{
    // advance time
    100::ms => now;
}
