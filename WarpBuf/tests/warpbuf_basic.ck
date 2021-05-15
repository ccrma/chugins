8 => int numQuarterNotes;  // 8 is two measures  
125. => float bpm;  
Phasor playhead => blackhole;
playhead.gain(numQuarterNotes);
bpm/(60.* numQuarterNotes) => playhead.freq;

playhead => WarpBuf s1 => dac;
s1.gain(.5);

// s1.setTimeRatio(2.); // play back in twice the amount of time.
s1.setTimeRatio(.5);  // play back in half the amount of time.

me.dir() + "assets/drums.wav" => s1.read;

s1.setTimeRatio(2.0);  // play back in half the amount of time.
s1.setPitchScale(1.5);  // play back in half the amount of time.

SinOsc timeLFO => blackhole;
0.1 => timeLFO.freq;

fun void modulateTimeRatio() {
	while (true) {
		0.5+1.5*(timeLFO.last()*0.5+.5) => s1.setTimeRatio;
		10::samp => now;
	}
}
spork~modulateTimeRatio();

while(true) {
	1::second => now;
}