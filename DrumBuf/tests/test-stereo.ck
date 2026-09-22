// Test stereo output of DrumBuf
// This test loads a stereo file and plays it to verify stereo functionality

DrumBuf sampler => dac;

// Load a stereo file
if (sampler.read(me.dir() + "/snare.wav") == 0) {
    <<< "FAIL: could not load snare.wav" >>>;
    me.exit();
}

// Print file info
<<< "File loaded:", sampler.filename() >>>;
<<< "Channels:", sampler.channels() >>>;
<<< "Sample rate:", sampler.samplerate() >>>;
<<< "Length:", sampler.length(), "seconds" >>>;
<<< "" >>>;

// Test stereo output
<<< "Playing stereo file - listen for stereo imaging" >>>;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();

<<< "" >>>;
<<< "Test complete!" >>>;
