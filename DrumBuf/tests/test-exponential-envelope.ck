//-----------------------------------------------------------------------------
// Test Exponential Envelope
// Tests the exponential AR envelope with various attack/release times
//-----------------------------------------------------------------------------

DrumBuf sampler => dac;

if (sampler.read(me.dir() + "/snare.wav") == 0) {
    <<< "FAIL: could not load snare.wav" >>>;
    me.exit();
}

<<< "=== Testing Exponential AR Envelope ===\n" >>>;

// Enable envelope
1 => sampler.envelope;

//-----------------------------------------------------------------------------
// Test 1: Very fast attack, slow release
//-----------------------------------------------------------------------------
<<< "Test 1: Very fast attack (0.001s), slow release (0.5s)" >>>;
<<< "Should hear a sharp attack followed by a slow fade-out" >>>;
0.001 => sampler.attack;
0.5 => sampler.release;

sampler.noteOn(60, 127);
200::ms => now;
sampler.noteOff();
700::ms => now;  // Wait for full release

//-----------------------------------------------------------------------------
// Test 2: Slow attack, fast release
//-----------------------------------------------------------------------------
<<< "\nTest 2: Slow attack (0.3s), fast release (0.05s)" >>>;
<<< "Should hear a gentle fade-in followed by a quick cut" >>>;
0.3 => sampler.attack;
0.05 => sampler.release;

sampler.noteOn(60, 127);
500::ms => now;
sampler.noteOff();
200::ms => now;

//-----------------------------------------------------------------------------
// Test 3: Medium attack and release
//-----------------------------------------------------------------------------
<<< "\nTest 3: Medium attack (0.05s), medium release (0.2s)" >>>;
<<< "Balanced envelope for natural drum response" >>>;
0.05 => sampler.attack;
0.2 => sampler.release;

sampler.noteOn(60, 127);
300::ms => now;
sampler.noteOff();
400::ms => now;

//-----------------------------------------------------------------------------
// Test 4: Instant attack, normal release
//-----------------------------------------------------------------------------
<<< "\nTest 4: Instant attack (0.0s), normal release (0.15s)" >>>;
<<< "Should hear full volume immediately, then smooth fade-out" >>>;
0.0 => sampler.attack;
0.15 => sampler.release;

sampler.noteOn(60, 127);
200::ms => now;
sampler.noteOff();
300::ms => now;

//-----------------------------------------------------------------------------
// Test 5: Normal attack, instant release
//-----------------------------------------------------------------------------
<<< "\nTest 5: Normal attack (0.02s), instant release (0.0s)" >>>;
<<< "Should hear smooth fade-in, then immediate cutoff" >>>;
0.02 => sampler.attack;
0.0 => sampler.release;

sampler.noteOn(60, 127);
200::ms => now;
sampler.noteOff();
100::ms => now;

//-----------------------------------------------------------------------------
// Test 6: Compare three different release times
//-----------------------------------------------------------------------------
<<< "\nTest 6: Comparing release times (0.05s, 0.2s, 0.8s)" >>>;
<<< "All with same attack (0.01s)" >>>;
0.01 => sampler.attack;

<<< "  6a: Short release (0.05s)" >>>;
0.05 => sampler.release;
sampler.noteOn(60, 127);
150::ms => now;
sampler.noteOff();
200::ms => now;

<<< "  6b: Medium release (0.2s)" >>>;
0.2 => sampler.release;
sampler.noteOn(60, 127);
150::ms => now;
sampler.noteOff();
400::ms => now;

<<< "  6c: Long release (0.8s)" >>>;
0.8 => sampler.release;
sampler.noteOn(60, 127);
150::ms => now;
sampler.noteOff();
1000::ms => now;

//-----------------------------------------------------------------------------
// Test 7: Rapid fire with envelope
//-----------------------------------------------------------------------------
<<< "\nTest 7: Rapid-fire notes with envelope (0.01s attack, 0.1s release)" >>>;
<<< "Tests envelope retriggering" >>>;
0.01 => sampler.attack;
0.1 => sampler.release;

for (0 => int i; i < 8; i++) {
    sampler.noteOn(60, 120 - (i * 10));  // Decreasing velocity
    100::ms => now;
    sampler.noteOff();
    50::ms => now;
}

//-----------------------------------------------------------------------------
// Test 8: Long sustain
//-----------------------------------------------------------------------------
<<< "\nTest 8: Long sustained note with envelope" >>>;
<<< "Attack (0.1s), sustain (1s), release (0.3s)" >>>;
0.1 => sampler.attack;
0.3 => sampler.release;

sampler.noteOn(60, 127);
1.2::second => now;  // Let attack complete and sustain
sampler.noteOff();
500::ms => now;

<<< "\n=== Exponential Envelope Test Complete ===" >>>;
<<< "The exponential curves should sound more natural than linear ramps." >>>;
<<< "Attack should feel smooth and gradual." >>>;
<<< "Release should feel like a natural decay." >>>;
