WarpBuf s1 => dac;
WarpBuf s2 => dac;

s1.gain(.4);
s2.gain(.4);

151. => float BPM;
BPM => s1.bpm;
BPM => s2.bpm;

me.dir() + "assets/drums.wav" => s1.read;
me.dir() + "assets/synth.wav" => s2.read;

2. => s2.transpose;

// 1 => s1.loop;
// 2. => s1.loopStart;
// 3. => s1.loopEnd;

while(true) {
	1::second => now;
}