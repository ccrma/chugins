// name: fin.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
adc => Faust moogVCF => dac;
// compile faust code
moogVCF.eval(`
import("effect.lib"); 
process = moog_vcf_demo ;
`);
// parameter dump
moogVCF.dump();

// time loop
while( true )
{
    // advance time
    100::ms => now;
}
