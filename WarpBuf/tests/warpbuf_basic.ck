fun void heartbeat() {
    while(true) {
        <<<"heartbeat">>>;
        500::ms => now;
    }
}
spork~ heartbeat();

WarpBuf s1 => dac;
WarpBuf s2 => dac;

s1.gain(.4);
s2.gain(.4);

float BPM;

fun void setBPM(float newBPM) {
    newBPM => BPM => s1.bpm => s2.bpm;
}

setBPM(140.);

me.dir() + "assets/1375__sleep__90-bpm-nylon2.wav" => s1.read;
me.dir() + "assets/381353__waveplaysfx__drumloop-120-bpm-edm-drum-loop-022.wav" => s2.read;

// test that loop off works.
// You should only hear the drum loop once
// and the guitar should loop.
// 0 => s2.loop;

// Test that ridiculous bounds don't break it.
// The guitar should be silent always.
// 1000. => s1.loopStart => s1.startMarker;
// 1000. => s1.loopEnd;
// 1000. => s1.playhead;

// Test that ridiculous bounds don't break it.
// The guitar should be silent always.
// 1000. => s1.loopStart;
// 999. => s1.loopEnd;
// 998. => s1.playhead;

// Test that ridiculous bounds don't break it.
// The guitar should be silent always.
// -2. => s1.loopStart;
// -1. => s1.loopEnd;
// 3. => s1.playhead;

// Test that ridiculous bounds don't break it.
// The guitar should be silent always.
// -2. => s1.loopStart;
// -1. => s1.loopEnd;
// -3. => s1.playhead;

while(true) {
	setBPM(110+20.*Math.random2(0,4));
	(240./BPM)::second => now;
	Std.rand2(-6, 0) => s2.transpose;
}