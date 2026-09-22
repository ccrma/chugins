//-----------------------------------------------------------------------------
// Test Time Stretching (should NOT change pitch)
//-----------------------------------------------------------------------------

DrumBuf sampler => dac;

if (sampler.read(me.dir() + "/SovereignBreak.wav") == 0) {
    <<< "FAIL: could not load SovereignBreak.wav" >>>;
    me.exit();
}

<<< "=== Testing Time Stretch (Rate) ===" >>>;
<<< "NOTE: Rate should change SPEED but NOT PITCH" >>>;
<<< "" >>>;

// Reference
<<< "Test 1: Normal speed (1.0x)" >>>;
1.0 => sampler.rate;
0.0 => sampler.transpose;  // Make sure no pitch shift
sampler.noteOn(60, 127);
3::second => now;
sampler.noteOff();
2::second => now;

// Slow down
<<< "Test 2: Half speed (0.5x)" >>>;
<<< "  Should take twice as long but SAME pitch" >>>;
0.5 => sampler.rate;
0.0 => sampler.transpose;
sampler.noteOn(60, 127);
5::second => now;
sampler.noteOff();
2::second => now;

// Speed up
<<< "Test 3: Double speed (2.0x)" >>>;
<<< "  Should take half as long but SAME pitch" >>>;
2.0 => sampler.rate;
0.0 => sampler.transpose;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
2::second => now;

// Very slow
<<< "Test 4: Quarter speed (0.25x)" >>>;
<<< "  Should be very slow but SAME pitch" >>>;
0.25 => sampler.rate;
0.0 => sampler.transpose;
sampler.noteOn(60, 127);
6::second => now;
sampler.noteOff();

<<< "" >>>;
<<< "Time stretch test complete" >>>;
<<< "If pitch changed, we need to fix the rate implementation" >>>;
