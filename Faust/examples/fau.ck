// name: fau.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
Faust fck => dac;

// evaluate Faust code
fck.eval(`
  freq=nentry("freq",440,50,2000,0.01);
  process=os.sawtooth(freq) <: _,_;
`);

// time loop
while( true )
{
  // set (will auto append /0x00/)
  fck.v( "freq", Math.random2f(400,800) );
  // print snapshot
  fck.dump();
  // advance time
  100::ms => now;
}
