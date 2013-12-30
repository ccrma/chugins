SndBuf obama => PitchTrack pt => blackhole;
512 => pt.frame;
4 => pt.overlap;

obama => Delay del => PitShift autotune => Envelope fade => dac;
1::second => fade.duration;
0 => fade.value;
1 => fade.target;

// uncomment these to produce a wav file of the output
dac => WvOut2 record => blackhole;
"obama_autotuned" => record.wavFilename;

pt.frame()::samp => del.delay;
"data/obama.wav" => obama.read;
0 => obama.pos;
obama.length() => dur length;

1 => autotune.mix;
1 => autotune.shift;

// Echoing bells matching vocal pitch
TriOsc tri => ADSR env => Echo e => dac;
env.set(5::ms, 250::ms, 0, 0::ms);
0.5 => e.mix;
e => e;
0.8 => e.gain;
250::ms => e.max => e.delay;
0.15 => tri.gain;

[0, 3, 7, 10] @=> int pentatonic[]; //Cm7 chord

float last_target;

now + length => time endtime;
now + (length - fade.duration()) => time fadetime;

spork ~ switchScale();
spork ~ ostinato();

while (now < endtime)
{
	samp => now;
	pt.get() => float track;
	
	// find closest pitch
	closest (track, pentatonic) => float target;

	// only trigger new note if different from previous
	if (target > 60 && Std.fabs(target - last_target) > 0.1)
	{
		1 => env.keyOn;
		target => tri.freq;
		target => last_target;
	}

	// perform autotune
	if (track > 0) target / track => autotune.shift;
	// begin fade out if the time is right
	if (now >= fadetime) 0 => fade.target;
}
// wait a little extra time
5::second => now;

fun void hold_pitch(float t)
{
	t => last_target;
	250::ms => now;
	-100 => last_target;
}

fun void switchScale()
{
	while (1)
	{
		12::second => now;
		pentatonic[0]--;
		pentatonic[2]--;
		pentatonic[3]++;
		11.5::second => now;
		pentatonic[0]--;
		2 -=> pentatonic[1];
		pentatonic[2]--;
		3 -=> pentatonic[3];
		for (int i; i<pentatonic.size(); i++)
		{
			if (pentatonic[i] < -5) 12 +=> pentatonic[i];
		}
	}
}

fun void ostinato()
{
	ModalBar mb => GVerb rev => fade;
	0.5 => mb.gain;
	1 => mb.preset;
	//0.01 => rev.mix;
	while (1)
	{
		for (int i; i<pentatonic.size(); i++)
		{
			(pentatonic[i] + 48) => Std.mtof => mb.freq;
			1 => mb.noteOn;
			125::ms => now;
		}
	}
}

// helper function to find equal tempered pitch in list
// which is closest to freq testval.
fun float closest (float testval, int list[])
{
	list.size() => int len;

	int octave;
	Std.ftom(testval) => float testmidi;
	while (testmidi - (list[len-1] + octave) > 12)
	{
		12 +=> octave;
	}

	48000.0 => float lowdiff;
	int closest_index;
	int closest_octave;

	for (int i; i<len; i++)
	{
		Std.mtof(octave + list[i]) => float listnote;
		Math.fabs(listnote - testval) => float diff;
		if (diff < lowdiff)
		{
			i => closest_index;
			diff => lowdiff;
			octave => closest_octave;
		}
	}

	for (int i; i<len; i++)
	{
		Std.mtof(octave + 12 + list[i]) => float listnote;
		Math.fabs(listnote - testval) => float diff;
		if (diff < lowdiff)
		{
			i => closest_index;
			diff => lowdiff;
			octave + 12 => closest_octave;
		}
	}
	
	return Std.mtof(closest_octave + list[closest_index]);
}
