//-----------------------------------------------------------------------------
// Test Ping-Pong Looping Mode
//-----------------------------------------------------------------------------

DrumBuf sampler => dac;

if (sampler.read(me.dir() + "/SovereignBreak.wav") == 0) {
    <<< "FAIL: could not load SovereignBreak.wav" >>>;
    me.exit();
}

<<< "=== Testing Ping-Pong Loop Mode ===" >>>;
<<< "" >>>;

// Test 1: Forward for reference
<<< "Test 1: Forward loop (reference)" >>>;
1 => sampler.playbackMode;  // LOOP
0.15 => sampler.loopStart;
0.35 => sampler.loopEnd;
sampler.noteOn(60, 127);
4::second => now;
sampler.noteOff();
2::second => now;

// Test 2: Ping-pong
<<< "Test 2: Ping-pong loop" >>>;
<<< "  (Should bounce back and forth)" >>>;
2 => sampler.playbackMode;  // PINGPONG
0.15 => sampler.loopStart;
0.35 => sampler.loopEnd;
sampler.noteOn(60, 127);
10::second => now;
sampler.noteOff();
2::second => now;

// Test 3: Ping-pong with faster rate
<<< "Test 3: Ping-pong (1.5x speed)" >>>;
<<< "  (Should bounce faster)" >>>;
1.5 => sampler.rate;
sampler.noteOn(60, 127);
8::second => now;
sampler.noteOff();
1.0 => sampler.rate;
2::second => now;

// Test 4: Ping-pong with pitch change
<<< "Test 4: Ping-pong with pitch (+7 semitones)" >>>;
7.0 => sampler.transpose;
sampler.noteOn(60, 127);
8::second => now;
sampler.noteOff();

<<< "" >>>;
<<< "Ping-pong test complete" >>>;
