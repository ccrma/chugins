// name: flanger.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
adc => Faust flanger => dac;

// evaluate Faust code
flanger.eval(`
  process = dm.flanger_demo ;
`);

// print the parameters of the Faust object
flanger.dump();

// time loop
while( true )
{
  // advance time
  100::ms => now;
}
