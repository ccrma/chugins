// name: file-playback.ck
// desc: Play a short audio file like a polyphonic "sampler"

// instantiate and connect faust => ck
Faust playback => dac;

5 => playback.numVoices;

me.dir() + "assets" => string assetsDir => playback.assetsDir;
<<< "assetsDir: ", assetsDir >>>;

// evaluate file playback
playback.compile("polyphonic_sampler.dsp");

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