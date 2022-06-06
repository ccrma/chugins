// name: file-playback.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
Faust playback => dac;

// evaluate file playback
playback.eval(`
	process = 0,_~+(1):soundfile("label[url:{'60988__folktelemetry__crash-fast-14.wav'}]",2):!,!,si.bus(2);
`);

// time loop
while( true )
{
	// advance time
	100::ms => now;
}
