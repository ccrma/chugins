// name: reverb.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
adc => Faust reverb => dac;

// evaluate Faust code
reverb.eval(`
  process = dm.zita_light ;
`);

// print the parameters of the Faust object
reverb.dump();

// time loop
while( true )
{
  // advance time
  100::ms => now;
}
