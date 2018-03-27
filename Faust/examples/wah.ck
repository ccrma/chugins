// name: wah.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
adc => Faust wah => dac;

// evaluate Faust code
wah.eval(`
  process = dm.wah4_demo <: _,_ ;
`);

// parameter dump
wah.dump();

// time loop
while( true )
{
  // advance time
  100::ms => now;
}
