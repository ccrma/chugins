// name: fauReverb.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
Faust fck => Faust reverb => dac;

// evaluate Faust synth
fck.eval(`
	freq=nentry("freq",440,50,2000,0.01);
	process = os.sawtooth(freq);
`);

// evaluate Faust reverb
reverb.eval(`
	process = _ <: dm.zita_light;
`);

// NOTE: alternatively, the synth and the reverb could have been combined
// in a single Faust object:
// fck.eval(`
// 	freq = nentry("freq",440,50,2000,0.01);
//	process = os.sawtooth(freq) <: dm.zita_light;
// `);

// time loop
while( true )
{
	// set
	fck.v("freq", Math.random2f(400,800) );
	// advance time
	100::ms => now;
}
