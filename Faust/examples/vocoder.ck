// name: vocoder.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
adc => Faust vocoder => dac;

// evaluate Faust code
vocoder.eval(`
  process = dm.vocoder_demo ;
`);

// parameter dump
vocoder.dump();

// time loop
while( true )
{
  // advance time
  100::ms => now;
}
