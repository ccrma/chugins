// name: file-playback.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
Faust playback;

me.dir() + "assets" => string assetsDir => playback.assetsDir;
<<< "assetsDir: ", assetsDir >>>;

5 => playback.nvoices;

// evaluate file playback
playback.compile("polyphonic_sampler.dsp");

playback => dac;

// time loop
while( true )
{
	Math.random2(60, 72) => int pitch;
	playback.noteOn(pitch, 127);
	// advance time
	100::ms => now;
	playback.noteOff(pitch, 0);
	100::ms => now;
}