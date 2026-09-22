//-----------------------------------------------------------------------------
// Test AR Envelope
//-----------------------------------------------------------------------------

DrumBuf sampler => dac;

if (sampler.read(me.dir() + "/SovereignBreak.wav") == 0) {
    <<< "FAIL: could not load SovereignBreak.wav" >>>;
    me.exit();
}

<<< "=== Testing AR Envelope ===" >>>;
<<< "" >>>;

// Test 1: Without envelope (default)
<<< "Test 1: No envelope (default)" >>>;
0 => sampler.envelope;  // Ensure it's off
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
1::second => now;

// Test 2: With envelope - fast attack/release
<<< "Test 2: Fast AR envelope (10ms attack, 50ms release)" >>>;
1 => sampler.envelope;  // Enable
0.01 => sampler.attack;   // 10ms attack
0.05 => sampler.release;  // 50ms release

sampler.noteOn(60, 127);
500::ms => now;  // Let attack complete
sampler.noteOff();
200::ms => now;  // Wait for release
1::second => now;

// Test 3: Slow attack
<<< "Test 3: Slow attack (500ms attack, 100ms release)" >>>;
0.5 => sampler.attack;   // 500ms attack
0.1 => sampler.release;  // 100ms release

sampler.noteOn(60, 127);
1::second => now;  // Hear the attack ramping up
sampler.noteOff();
300::ms => now;  // Wait for release
1::second => now;

// Test 4: Long release
<<< "Test 4: Fast attack, long release (10ms attack, 1s release)" >>>;
0.01 => sampler.attack;   // 10ms attack
1.0 => sampler.release;   // 1 second release

sampler.noteOn(60, 127);
200::ms => now;  // Brief note
sampler.noteOff();
<<< "Listen to the 1 second release tail..." >>>;
1.5::second => now;
1::second => now;

// Test 5: Multiple notes with envelope (percussive)
<<< "Test 5: Percussive pattern with envelope" >>>;
0.005 => sampler.attack;   // 5ms attack
0.2 => sampler.release;    // 200ms release

for (0 => int i; i < 8; i++) {
    sampler.noteOn(60, 100);
    50::ms => now;
    sampler.noteOff();
    200::ms => now;
}

1::second => now;

// Test 6: Envelope with looping
<<< "Test 6: Envelope with looping (attack controls fade-in)" >>>;
1 => sampler.playbackMode;  // LOOP
0.2 => sampler.loopStart;
0.5 => sampler.loopEnd;
0.3 => sampler.attack;   // 300ms attack for smooth fade-in
0.2 => sampler.release;

sampler.noteOn(60, 127);
2::second => now;  // Loops while sustaining
sampler.noteOff();
<<< "Release while looping..." >>>;
500::ms => now;

// Reset
0 => sampler.playbackMode;  // ONESHOT
0 => sampler.envelope;

<<< "" >>>;
<<< "Envelope test complete" >>>;
<<< "" >>>;
<<< "AR Envelope behavior:" >>>;
<<< "  - ATTACK: Fades in from 0 to 1 over attack time" >>>;
<<< "  - SUSTAIN: Holds at 1.0 until noteOff()" >>>;
<<< "  - RELEASE: Fades out from current level to 0 over release time" >>>;
<<< "  - When envelope reaches 0, playback stops automatically" >>>;
