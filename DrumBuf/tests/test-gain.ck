//-----------------------------------------------------------------------------
// Test Gain Control
//-----------------------------------------------------------------------------

DrumBuf sampler => dac;

if (sampler.read(me.dir() + "/SovereignBreak.wav") == 0) {
    <<< "FAIL: could not load SovereignBreak.wav" >>>;
    me.exit();
}

<<< "=== Testing Gain Control (in dB) ===" >>>;
<<< "" >>>;

// Test 1: Unity gain (0 dB)
<<< "Test 1: Unity gain (0 dB)" >>>;
0.0 => sampler.gain;
<<< "Current gain:", sampler.gain(), "dB" >>>;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
1::second => now;

// Test 2: Reduced gain (-6 dB = half volume)
<<< "Test 2: -6 dB (half volume)" >>>;
-6.0 => sampler.gain;
<<< "Current gain:", sampler.gain(), "dB" >>>;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
1::second => now;

// Test 3: Boosted gain (+6 dB = double volume)
<<< "Test 3: +6 dB (double volume)" >>>;
6.0 => sampler.gain;
<<< "Current gain:", sampler.gain(), "dB" >>>;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
1::second => now;

// Test 4: Very quiet (-12 dB)
<<< "Test 4: -12 dB (very quiet)" >>>;
-12.0 => sampler.gain;
<<< "Current gain:", sampler.gain(), "dB" >>>;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
1::second => now;

// Test 5: Very loud (+12 dB)
<<< "Test 5: +12 dB (very loud)" >>>;
12.0 => sampler.gain;
<<< "Current gain:", sampler.gain(), "dB" >>>;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();

<<< "" >>>;
<<< "Gain control test complete" >>>;
<<< "Note: 0 dB = unity gain" >>>;
<<< "      -6 dB ≈ half volume" >>>;
<<< "      +6 dB ≈ double volume" >>>;
