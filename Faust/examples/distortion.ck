// name: distortion.ck
// desc: demo of Faust chugin in action!

// instantiate and connect
adc => Faust distortion => dac;

// evaluate Faust code
distortion.eval(`
  process = dm.cubicnl_demo <: _,_;
`);

// print the parameters of the Faust object
distortion.dump();

// mild distortion
distortion.v("/CUBIC_NONLINEARITY_cubicnl/Drive",0.5);

// time loop
while( true )
{
  // advance time
  100::ms => now;
}
