//-----------------------------------------------------------------------------
// Test Instant Attack/Release (0.0 seconds)
//-----------------------------------------------------------------------------

DrumBuf sampler => dac;

if (sampler.read(me.dir() + "/snare.wav") == 0) {
    <<< "FAIL: could not load snare.wav" >>>;
    me.exit();
}

<<< "=== Testing Instant Envelope (0.0 attack/release) ===" >>>;
<<< "" >>>;

// Enable envelope
1 => sampler.envelope;

//-----------------------------------------------------------------------------
// Test 1: Instant attack (0.0s)
//-----------------------------------------------------------------------------
<<< "Test 1: Instant attack (0.0s), normal release (0.1s)" >>>;
0.0 => sampler.attack;   // Instant attack
0.1 => sampler.release;

sampler.noteOn(60, 127);
200::ms => now;
sampler.noteOff();
200::ms => now;

//-----------------------------------------------------------------------------
// Test 2: Normal attack, instant release (0.0s)
//-----------------------------------------------------------------------------
<<< "Test 2: Normal attack (0.01s), instant release (0.0s)" >>>;
0.01 => sampler.attack;
0.0 => sampler.release;   // Instant release

sampler.noteOn(60, 127);
200::ms => now;
sampler.noteOff();
200::ms => now;

//-----------------------------------------------------------------------------
// Test 3: Both instant (0.0s)
//-----------------------------------------------------------------------------
<<< "Test 3: Instant attack (0.0s) and instant release (0.0s)" >>>;
0.0 => sampler.attack;
0.0 => sampler.release;

sampler.noteOn(60, 127);
200::ms => now;
sampler.noteOff();
200::ms => now;

//-----------------------------------------------------------------------------
// Test 4: Compare with non-zero times
//-----------------------------------------------------------------------------
<<< "Test 4: Non-instant for comparison - attack=0.05s, release=0.2s" >>>;
0.05 => sampler.attack;
0.2 => sampler.release;

sampler.noteOn(60, 127);
200::ms => now;
sampler.noteOff();
300::ms => now;

<<< "" >>>;
<<< "=== Instant Envelope Test Complete ===" >>>;
<<< "" >>>;
<<< "Instant attack (0.0s) should have no fade-in" >>>;
<<< "Instant release (0.0s) should cut off immediately on noteOff" >>>;
