//-----------------------------------------------------------------------------
// DrumBuf Test Script
// Tests all features of the DrumBuf chugin
//-----------------------------------------------------------------------------
// To run this test:
//   chuck --chugin:DrumBuf.chug DrumBuf-test.ck
// Or if installed in your chugins path:
//   chuck DrumBuf-test.ck
//-----------------------------------------------------------------------------

// Create a DrumBuf instance
DrumBuf sampler => dac;

// Load a test audio file
// NOTE: Replace with your own audio file path
// Supported formats: WAV, FLAC, OGG, etc (anything libsndfile supports)
if (sampler.read(me.dir() + "/SovereignBreak.wav") == 0) {
    <<< "FAIL: could not load SovereignBreak.wav" >>>;
    <<< "You can use any audio file supported by libsndfile (WAV, FLAC, OGG, MP3, etc.)" >>>;
    me.exit();
}

<<< "=== DrumBuf Test Suite ===" >>>;
<<< "" >>>;

//-----------------------------------------------------------------------------
// Test 1: Basic playback with noteOn
//-----------------------------------------------------------------------------
<<< "Test 1: Basic playback (3 seconds)" >>>;
sampler.noteOn(60, 127);  // Trigger at MIDI note 60, full velocity
3::second => now;
sampler.noteOff();
1::second => now;

//-----------------------------------------------------------------------------
// Test 2: Pitch transposition
//-----------------------------------------------------------------------------
<<< "Test 2: Pitch transposition" >>>;
<<< "  Playing at +5 semitones" >>>;
5.0 => sampler.transpose;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
1::second => now;

<<< "  Playing at -7 semitones" >>>;
-7.0 => sampler.transpose;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
0.0 => sampler.transpose;  // Reset
1::second => now;

//-----------------------------------------------------------------------------
// Test 3: Time stretching (rate control)
//-----------------------------------------------------------------------------
<<< "Test 3: Time stretching" >>>;
<<< "  Playing at half speed" >>>;
0.5 => sampler.rate;
sampler.noteOn(60, 127);
3::second => now;
sampler.noteOff();
1::second => now;

<<< "  Playing at double speed" >>>;
2.0 => sampler.rate;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
1.0 => sampler.rate;  // Reset
1::second => now;

//-----------------------------------------------------------------------------
// Test 4: Grain size adjustment
//-----------------------------------------------------------------------------
<<< "Test 4: Grain size" >>>;
<<< "  Tiny grains (10ms)" >>>;
0.01 => sampler.grainSize;  // 10ms
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
1::second => now;

<<< "  Large grains (200ms)" >>>;
0.2 => sampler.grainSize;  // 200ms
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
0.05 => sampler.grainSize;  // Reset to default
1::second => now;

//-----------------------------------------------------------------------------
// Test 5: Looping (forward)
//-----------------------------------------------------------------------------
<<< "Test 5: Forward looping" >>>;
1 => sampler.playbackMode;  // LOOP
0.2 => sampler.loopStart;
0.4 => sampler.loopEnd;
sampler.noteOn(60, 100);
4::second => now;
sampler.noteOff();
0 => sampler.playbackMode;  // Reset to ONESHOT
1::second => now;

//-----------------------------------------------------------------------------
// Test 6: Reverse looping
//-----------------------------------------------------------------------------
<<< "Test 6: Reverse looping" >>>;
1 => sampler.playbackMode;  // LOOP
1 => sampler.reverse;
0.3 => sampler.loopStart;
0.6 => sampler.loopEnd;
sampler.noteOn(60, 100);
4::second => now;
sampler.noteOff();
0 => sampler.playbackMode;  // Reset to ONESHOT
0 => sampler.reverse;
1::second => now;

//-----------------------------------------------------------------------------
// Test 7: Ping-pong looping
//-----------------------------------------------------------------------------
<<< "Test 7: Ping-pong looping" >>>;
2 => sampler.playbackMode;  // PINGPONG
0.1 => sampler.loopStart;
0.5 => sampler.loopEnd;
sampler.noteOn(60, 100);
5::second => now;
sampler.noteOff();
0 => sampler.playbackMode;  // Reset to ONESHOT
0.0 => sampler.loopStart;  // Reset loop points
1.0 => sampler.loopEnd;
1::second => now;

//-----------------------------------------------------------------------------
// Test 8: MIDI note mapping
//-----------------------------------------------------------------------------
<<< "Test 8: MIDI note position mapping" >>>;
// Map different MIDI notes to different start positions in seconds
sampler.notePos(60, 0.0);   // C4 -> 0.0 seconds
sampler.notePos(62, 0.25);  // D4 -> 0.25 seconds
sampler.notePos(64, 0.5);   // E4 -> 0.5 seconds
sampler.notePos(65, 0.75);  // F4 -> 0.75 seconds

<<< "  Playing from different start positions" >>>;
sampler.noteOn(60, 100);  // From 0.0s
1::second => now;
sampler.noteOff();
500::ms => now;

sampler.noteOn(62, 100);  // From 0.25s
1::second => now;
sampler.noteOff();
500::ms => now;

sampler.noteOn(64, 100);  // From 0.5s
1::second => now;
sampler.noteOff();
500::ms => now;

sampler.noteOn(65, 100);  // From 0.75s
1::second => now;
sampler.noteOff();
1::second => now;

//-----------------------------------------------------------------------------
// Test 9: Velocity sensitivity
//-----------------------------------------------------------------------------
<<< "Test 9: Velocity sensitivity" >>>;
<<< "  Low velocity (40)" >>>;
sampler.noteOn(60, 40);
1.5::second => now;
sampler.noteOff();
500::ms => now;

<<< "  High velocity (127)" >>>;
sampler.noteOn(60, 127);
1.5::second => now;
sampler.noteOff();
1::second => now;

//-----------------------------------------------------------------------------
// Test 10: Combined effects (pitch + rate + looping)
//-----------------------------------------------------------------------------
<<< "Test 10: Combined effects" >>>;
<<< "  Pitch down + slow + ping-pong loop" >>>;
-5.0 => sampler.transpose;
0.7 => sampler.rate;
2 => sampler.playbackMode;  // PINGPONG
0.2 => sampler.loopStart;
0.6 => sampler.loopEnd;
0.03 => sampler.grainSize;  // Smaller grains for smoother sound

sampler.noteOn(60, 120);
6::second => now;
sampler.noteOff();

<<< "" >>>;
<<< "=== All tests completed! ===" >>>;
<<< "" >>>;
<<< "DrumBuf features tested:" >>>;
<<< "  ✓ Audio file loading" >>>;
<<< "  ✓ Note triggering (noteOn/noteOff)" >>>;
<<< "  ✓ Pitch transposition" >>>;
<<< "  ✓ Time stretching (rate control)" >>>;
<<< "  ✓ Grain size adjustment" >>>;
<<< "  ✓ Forward looping" >>>;
<<< "  ✓ Reverse looping" >>>;
<<< "  ✓ Ping-pong looping" >>>;
<<< "  ✓ MIDI note position mapping" >>>;
<<< "  ✓ Velocity sensitivity" >>>;
<<< "  ✓ Combined effects" >>>;
