//-----------------------------------------------------------------------------
// Test query methods (matching SndBuf API)
//-----------------------------------------------------------------------------

DrumBuf sampler => dac;

// Test before loading
<<< "=== Before loading ===" >>>;
<<< "  samplerate():", sampler.samplerate(), "Hz" >>>;
<<< "  samples():", sampler.samples() >>>;
<<< "  length():", sampler.length(), "seconds" >>>;
<<< "  channels():", sampler.channels() >>>;

// Load file
sampler.read(me.dir() + "/snare.wav");

// Test after loading
<<< "" >>>;
<<< "=== After loading ===" >>>;
<<< "  samplerate():", sampler.samplerate(), "Hz" >>>;
<<< "  samples():", sampler.samples() >>>;
<<< "  length():", sampler.length(), "seconds" >>>;
<<< "  channels():", sampler.channels() >>>;
<<< "  isLoaded():", sampler.isLoaded() >>>;
<<< "  filename():", sampler.filename() >>>;

// Verify calculations
<<< "" >>>;
<<< "=== Verification ===" >>>;
<<< "  samples / samplerate =", sampler.samples() / sampler.samplerate(), "seconds" >>>;
<<< "  (should match length):", sampler.length(), "seconds" >>>;

// Play it to verify it still works
<<< "" >>>;
<<< "Playing sample..." >>>;
sampler.noteOn(60, 127);
sampler.length()::second => now;
