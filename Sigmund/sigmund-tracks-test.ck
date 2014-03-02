// Analyze audio file
SndBuf obama => Sigmund siggy => blackhole;
// change this to the path to your audio file
"../PitchTrack/data/obama.wav" => obama.read;
0 => obama.pos;
true => siggy.tracks; // sort sinusoids into tracks
128 => Std.mtof => siggy.maxfreq; // don't track high freqs

10 => int numTracks;

numTracks => siggy.npeak; // set max number of peaks
2048 => siggy.npts; // larger analysis window

// resynthesize sound based on sinusoidal tracking
Envelope freq[numTracks]; // envelopes help smooth results
Envelope amp[numTracks];
SinOsc resynth[numTracks];
for (int i; i<numTracks; i++)
{
	resynth[i] => amp[i] => dac;
	freq[i] => blackhole;
	10::ms => freq[i].duration; // increase this for portamento
	10::ms => amp[i].duration;
}

obama.length() => dur len;
now + len => time end;

spork ~ updateFreqs();

while (now < end)
{
	for (int i; i<numTracks; i++)
	{
		siggy.peak(i) => freq[i].target;

		// uncomment this for funkier output
		//Std.ftom(siggy.peak(i))$int => Std.mtof => freq[i].target;
		siggy.amp(i) => amp[i].target;
	}
	// try a higher value for an audible pulse
	100::ms => now;
}

// freq envelope used to set resynth frequencies smoothly
fun void updateFreqs()
{
	while (true)
	{
		for (int i; i<numTracks; i++)
		{
			freq[i].value() => resynth[i].freq;
		}
		ms => now;
	}
}