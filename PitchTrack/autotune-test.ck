SndBuf obama => PitchTrack pt => blackhole;
obama => Delay del => PitShift autotune => dac;
//dac => WvOut2 record => blackhole;
//"obama_autotuned" => record.wavFilename;
1024::samp => del.delay;
1 => autotune.mix;
// change this to your own local file path
"/home/jwmatthys/Music/obama.wav" => obama.read;
0 => obama.pos;
1 => autotune.gain;
1 => autotune.shift;
TriOsc tri => ADSR env => Echo e => dac;
env.set(0::ms, 250::ms, 0, 1::ms);
0.5 => e.mix;
e => e;
0.8 => e.gain;
250::ms => e.delay;
0.1 => tri.gain;
1024 => pt.frame;
[0, 3, 7, 10] @=> int pentatonic[];

spork ~ switchScale();
spork ~ ostinato();

float last_target;

while (1)
{
	samp => now;
	pt.get() => float track;
	closest (track, pentatonic) => float target;
	if (Std.fabs(target - last_target) > 0.1)
	{
		1 => env.keyOn;
		target => tri.freq;
	}
	target => last_target;
	if (track > 0) target / track => autotune.shift;
}

fun void switchScale()
{
	while (1)
	{
		[0, 3, 7, 10] @=> pentatonic;
		12::second => now;
		[-1, 3, 6, 11] @=> pentatonic;
		11.5::second => now;
	}
}

fun void ostinato()
{
	ModalBar mb => GVerb rev => dac;
	0.25 => mb.gain;
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
