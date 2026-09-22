//-----------------------------------------------------------------------------
// DrumBuf Debug Script - Minimal test
//-----------------------------------------------------------------------------

DrumBuf sampler => dac;

// Load file
if (sampler.read(me.dir() + "/SovereignBreak.wav") == 0) {
    <<< "FAIL: could not load SovereignBreak.wav" >>>;
    me.exit();
}

<<< "=== DrumBuf Debug Test ===" >>>;
<<< "File loaded successfully" >>>;

// Test 1: Very basic playback with large grain size
<<< "Test 1: Large grains (500ms)" >>>;
0.5 => sampler.grainSize;  // 500ms - very large
sampler.noteOn(60, 127);
5::second => now;
sampler.noteOff();
2::second => now;

// Test 2: Medium grains
<<< "Test 2: Medium grains (100ms)" >>>;
0.1 => sampler.grainSize;
sampler.noteOn(60, 127);
5::second => now;
sampler.noteOff();
2::second => now;

// Test 3: Default grains
<<< "Test 3: Default grains (50ms)" >>>;
0.05 => sampler.grainSize;
sampler.noteOn(60, 127);
5::second => now;
sampler.noteOff();
2::second => now;

// Test 4: With looping
<<< "Test 4: Forward loop (50ms grains)" >>>;
1 => sampler.playbackMode;  // LOOP
0.0 => sampler.loopStart;
0.3 => sampler.loopEnd;  // Loop first 30% of file
sampler.noteOn(60, 127);
5::second => now;
sampler.noteOff();
0 => sampler.playbackMode;  // ONESHOT
1::second => now;

// Test 5: Pitch shifting (transpose)
<<< "Test 5: Pitch shifting" >>>;
0.05 => sampler.grainSize;  // Reset to default

<<< "  Original pitch" >>>;
0.0 => sampler.transpose;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
1::second => now;

<<< "  +7 semitones (perfect fifth up)" >>>;
7.0 => sampler.transpose;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
1::second => now;

<<< "  -12 semitones (one octave down)" >>>;
-12.0 => sampler.transpose;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
1::second => now;

<<< "  +12 semitones (one octave up)" >>>;
12.0 => sampler.transpose;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
0.0 => sampler.transpose;  // Reset
1::second => now;

// Test 6: Time stretching (rate control)
<<< "Test 6: Time stretching (independent of pitch)" >>>;

<<< "  Normal speed" >>>;
1.0 => sampler.rate;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
1::second => now;

<<< "  Half speed (0.5x)" >>>;
0.5 => sampler.rate;
sampler.noteOn(60, 127);
3::second => now;
sampler.noteOff();
1::second => now;

<<< "  Double speed (2.0x)" >>>;
2.0 => sampler.rate;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
1::second => now;

<<< "  Very slow (0.25x)" >>>;
0.25 => sampler.rate;
sampler.noteOn(60, 127);
3::second => now;
sampler.noteOff();
1.0 => sampler.rate;  // Reset
1::second => now;

// Test 7: Combined pitch + rate
<<< "Test 7: Combined pitch and time stretch" >>>;

<<< "  Pitch up (+5 semitones) + slow down (0.7x)" >>>;
5.0 => sampler.transpose;
0.7 => sampler.rate;
sampler.noteOn(60, 127);
3::second => now;
sampler.noteOff();
1::second => now;

<<< "  Pitch down (-5 semitones) + speed up (1.5x)" >>>;
-5.0 => sampler.transpose;
1.5 => sampler.rate;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();

// Reset
0.0 => sampler.transpose;
1.0 => sampler.rate;
1::second => now;

// Test 8: Reverse looping
<<< "Test 8: Reverse looping" >>>;
1 => sampler.playbackMode;  // LOOP
1 => sampler.reverse;
0.2 => sampler.loopStart;
0.8 => sampler.loopEnd;

<<< "  Reverse loop (normal speed)" >>>;
sampler.noteOn(60, 127);
5::second => now;
sampler.noteOff();
1::second => now;

<<< "  Reverse loop (slow 0.5x)" >>>;
0.5 => sampler.rate;
sampler.noteOn(60, 127);
5::second => now;
sampler.noteOff();
1.0 => sampler.rate;
0 => sampler.reverse;  // Reset reverse
1::second => now;

// Test 9: Ping-pong looping
<<< "Test 9: Ping-pong looping" >>>;
2 => sampler.playbackMode;  // PINGPONG
0.1 => sampler.loopStart;
0.4 => sampler.loopEnd;

<<< "  Ping-pong (normal speed)" >>>;
sampler.noteOn(60, 127);
8::second => now;
sampler.noteOff();
1::second => now;

<<< "  Ping-pong (fast 1.5x)" >>>;
1.5 => sampler.rate;
sampler.noteOn(60, 127);
6::second => now;
sampler.noteOff();
1.0 => sampler.rate;
1::second => now;

<<< "  Ping-pong with pitch (+5 semitones)" >>>;
5.0 => sampler.transpose;
sampler.noteOn(60, 127);
6::second => now;
sampler.noteOff();
0.0 => sampler.transpose;

// Reset all
0 => sampler.playbackMode;  // ONESHOT
0.0 => sampler.loopStart;
1.0 => sampler.loopEnd;

<<< "" >>>;
<<< "Debug test complete" >>>;
