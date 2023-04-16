// name: sidechain-compressor
// desc: Reduce the volume of the first stereo input
//       (a sine tone) according to the loudness of
//       the second (a looping pluck).
Faust faust => dac;

faust.compile("sidechain.dsp");

faust.dump();

SinOsc sinOsc;
220. => sinOsc.freq;

SndBuf2 buf;
1 => buf.loop;
1. => buf.gain;

"special:mandpluk" => buf.read;

// the aux channels that go into the level detector
buf.chan(0) => faust.chan(0);
buf.chan(1) => faust.chan(1);

// the main channels whose gains get adjusted
sinOsc.chan(0) => faust.chan(2);
sinOsc.chan(0) => faust.chan(3);

faust.v("/COMPRESSOR/Settings/Threshold", -30.);
faust.v("/COMPRESSOR/Settings/Attack", 5.);
faust.v("/COMPRESSOR/Settings/Release", 40.);

while(true) {
	100::ms => now;
}

