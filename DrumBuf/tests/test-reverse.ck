//-----------------------------------------------------------------------------
// Test Reverse Playback
// Tests the reverse flag for both one-shot and looped playback
//-----------------------------------------------------------------------------

DrumBuf sampler => dac;

if (sampler.read(me.dir() + "/SovereignBreak.wav") == 0) {
    <<< "FAIL: could not load SovereignBreak.wav" >>>;
    me.exit();
}

<<< "=== Testing Reverse Playback ===" >>>;
<<< "" >>>;

// Test 1: Forward one-shot (reference)
<<< "Test 1: Forward one-shot (reference)" >>>;
0 => sampler.reverse;
0 => sampler.playbackMode;  // ONESHOT
sampler.noteOn(60, 127);
3::second => now;
sampler.noteOff();
1::second => now;

// Test 2: Reverse one-shot
<<< "Test 2: Reverse one-shot" >>>;
<<< "  (Should play backward from end to start)" >>>;
1 => sampler.reverse;
0 => sampler.playbackMode;  // ONESHOT
sampler.noteOn(60, 127);
3::second => now;
sampler.noteOff();
1::second => now;

// Test 3: Forward loop (reference)
<<< "Test 3: Forward loop (reference)" >>>;
0 => sampler.reverse;
1 => sampler.playbackMode;  // LOOP
0.2 => sampler.loopStart;
0.6 => sampler.loopEnd;
sampler.noteOn(60, 127);
5::second => now;
sampler.noteOff();
1::second => now;

// Test 4: Reverse loop
<<< "Test 4: Reverse loop" >>>;
<<< "  (Should loop backward from end to start)" >>>;
1 => sampler.reverse;
1 => sampler.playbackMode;  // LOOP
0.2 => sampler.loopStart;
0.6 => sampler.loopEnd;
sampler.noteOn(60, 127);
5::second => now;
sampler.noteOff();
1::second => now;

// Test 5: Reverse with slower speed
<<< "Test 5: Reverse loop (0.5x speed)" >>>;
0.5 => sampler.rate;
1 => sampler.reverse;
1 => sampler.playbackMode;  // LOOP
sampler.noteOn(60, 127);
8::second => now;
sampler.noteOff();

<<< "" >>>;
<<< "Reverse test complete" >>>;
<<< "Note: reverse flag controls playback direction independently of loop mode" >>>;
