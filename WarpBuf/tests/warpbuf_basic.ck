WarpBuf s1 => dac;
WarpBuf s2 => dac;

s1.gain(.4);
s2.gain(.4);

151. => float BPM;
s1.setBPM(BPM);
s2.setBPM(BPM);

me.dir() + "assets/drums.wav" => s1.read;
me.dir() + "assets/synth.wav" => s2.read;

s2.setTranspose(2.);

while(true) {
	1::second => now;
}