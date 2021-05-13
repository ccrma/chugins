me.dir() + "assets/guitar.wav" => string file_path; // Must be absolute path.

Sampler sampler => dac;
sampler.gain(0.3);

<<< "Trying sound asset at path: ", file_path >>>;

if (sampler.read(file_path)) {
	<<< "loaded file!" >>>;
}
else {
	<<< "error reading file!" >>>;
}

for (0 => int i; i < sampler.getNumParameters(); 1 +=> i) {
	<<< "parameter ", i, " is named: ", sampler.getParameterName(i) >>>;
}

sampler.setNumVoices(20);     // 20 by default (30 max)
sampler.setVoiceStealing(1);  // voice stealing is disabled by default.

sampler.setParameter(0, 60);  // set to middle C
sampler.setParameter(1, 1);   // enable amp env active
sampler.setParameter(2, 2);   // amp env attack (ms)
sampler.setParameter(3, 10);  // amp env decay (ms)
sampler.setParameter(4, 1);   // amp env sustain
sampler.setParameter(5, 100); // amp env release (ms)
sampler.setParameter(8, 0);   // disable filter Active

[0, 2, 4, 5, 7, 9, 11, 12] @=> int notes[];

while(true) {

	repeat(10) {
		60 + notes[Std.rand2(0, notes.size()-1)] => int pitch;
	    sampler.noteOn(pitch, 1.);
	    30::ms => now;
	    sampler.noteOff(pitch, 0.);
	    Std.rand2(1,2)*30::ms => now;
	}

	sampler.setParameter(1, 1-sampler.getParameter(1)); // toggle amp envelope active
}
