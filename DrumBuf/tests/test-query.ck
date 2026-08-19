//-----------------------------------------------------------------------------
// Test Query Methods
// Tests length(), channels(), samplerate(), samples(), isLoaded(), filename()
//-----------------------------------------------------------------------------

<<< "=== Testing Query Methods ===" >>>;
<<< "" >>>;

DrumBuf sampler => dac;

// Test 1: Query before loading (should return defaults)
<<< "Test 1: Query before loading" >>>;
<<< "  isLoaded():", sampler.isLoaded() >>>;
<<< "  length():", sampler.length(), "seconds" >>>;
<<< "  channels():", sampler.channels() >>>;
<<< "  samplerate():", sampler.samplerate(), "Hz" >>>;
<<< "  filename():", sampler.filename() >>>;
<<< "" >>>;

// Test 2: Load file and query
<<< "Test 2: Query after loading SovereignBreak.wav" >>>;
if (sampler.read(me.dir() + "/SovereignBreak.wav") == 0) {
    <<< "FAIL: could not load SovereignBreak.wav" >>>;
    me.exit();
}

<<< "  isLoaded():", sampler.isLoaded() >>>;
<<< "  length():", sampler.length(), "seconds" >>>;
<<< "  channels():", sampler.channels() >>>;
<<< "  samplerate():", sampler.samplerate(), "Hz" >>>;
<<< "  filename():", sampler.filename() >>>;
<<< "" >>>;

// Test 3: Verify length is accurate
<<< "Test 3: Verify length accuracy" >>>;
<<< "  File should be ~2.98 seconds" >>>;
sampler.length() => float len;
if (len > 2.97 && len < 2.99) {
    <<< "  ✓ Length is correct:", len >>>;
} else {
    <<< "  ✗ Length seems wrong:", len >>>;
}
<<< "" >>>;

// Test 4: Constructor with filename
<<< "Test 4: Query with constructor-loaded file" >>>;
DrumBuf sampler2(me.dir() + "/snare.wav") => dac;
<<< "  isLoaded():", sampler2.isLoaded() >>>;
<<< "  length():", sampler2.length(), "seconds" >>>;
<<< "  channels():", sampler2.channels() >>>;
<<< "  samplerate():", sampler2.samplerate(), "Hz" >>>;
<<< "  filename():", sampler2.filename() >>>;
<<< "" >>>;

<<< "=== Query Methods Test Complete ===" >>>;
