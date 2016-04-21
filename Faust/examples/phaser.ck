// name: fin.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
adc => Faust phaser => dac;
// compile faust code
phaser.eval(`
import("effect.lib"); 
process = phaser2_demo ;
`);
// parameter dump
phaser.dump();

// time loop
while( true )
{
    // advance time
    100::ms => now;
}
