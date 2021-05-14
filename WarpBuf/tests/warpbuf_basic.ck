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

while(true) { 1::second => now; }