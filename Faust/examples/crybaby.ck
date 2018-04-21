// name: crybaby.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
adc => Faust cryBaby => dac;

// evaluate Faust code
cryBaby.eval(`
  process = dm.crybaby_demo <: _,_;
`);

// print the parameters of the Faust object
cryBaby.dump();

// ChucK LFO
SinOsc LFO => blackhole;
1 => LFO.freq;

// time loop
while( true )
{
  // oscillate
  cryBaby.v( "/CRYBABY/Wah_parameter", LFO.last() );
  // advance time
  5::ms => now;
}
