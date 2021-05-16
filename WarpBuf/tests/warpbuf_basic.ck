8 => int numQuarterNotes;  // 8 is two measures  
125. => float bpm;  
Phasor playhead => blackhole;
playhead.gain(numQuarterNotes);
bpm/(60.* numQuarterNotes) => playhead.freq;

playhead => WarpBuf s1 => dac;
playhead => WarpBuf s2 => dac;
s1.gain(.4);
s2.gain(.4);

me.dir() + "assets/drums.wav" => s1.read;
me.dir() + "assets/synth.wav" => s2.read;

144. => float BPM;

s1.setBPM(BPM);
s2.setBPM(BPM);

fun float semiToRatio(float semi) {
	return Math.mtof(60. + semi) / Math.mtof(60.);
}

s2.setPitchScale(semiToRatio(3.));

// SinOsc timeLFO => blackhole;
// 0.1 => timeLFO.freq;

// fun void modulateTimeRatio() {
// 	while (true) {
// 		0.5+1.5*(timeLFO.last()*0.5+.5) => s1.setTimeRatio;
// 		10::samp => now;
// 	}
// }
// spork~modulateTimeRatio();

while(true) {
	1::second => now;
}