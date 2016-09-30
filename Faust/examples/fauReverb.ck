// name: fau.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
Faust fck => Faust reverb => dac;
// evaluate faust code
fck.eval(`
	freq = button("freq");
	process = os.sawtooth(freq);
`);
// _<: and :> turn zita_rev_fdn_demo into a mono object
reverb.eval(`
	process = dm.zita_rev_fdn_demo;
`);

// time loop
while( true )
{
    // set
    fck.v("/0x00/freq", Math.random2f(400,800) );
    // advance time
    100::ms => now;
}
