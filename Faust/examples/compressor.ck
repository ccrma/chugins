// name: compressor.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
adc => Faust compressor => dac;

// evaluate Faust code
compressor.eval(` 
process = dm.compressor_demo;
`);

// print the parameters of the Faust object
compressor.dump();

// time loop
while( true )
{
  // advance time
  100::ms => now;
}
