// name: fau.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
Faust fck => dac;
// evaluate FAUST code
// -- (use ` to avoid escaping " and ')
// -- (auto import libs: stdfaust)
fck.eval(`
  freq=button("freq");
  process=os.sawtooth(freq)<:_,_;
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
