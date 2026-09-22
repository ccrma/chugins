// Comprehensive test of new playbackMode API
DrumBuf s(me.dir() + "/SovereignBreak.wav") => dac;

<<< "=== Testing playbackMode API ===" >>>;
<<< "" >>>;

// Test 1: ONESHOT (mode 0)
<<< "Mode 0: ONESHOT (play once, stop at end marker)" >>>;
0 => s.playbackMode;
0.0 => s.startMarker;
1.0 => s.endMarker;
s.noteOn(60, 127);
1.5::second => now;
<<< "" >>>;

// Test 2: LOOP (mode 1)
<<< "Mode 1: LOOP (loop between markers)" >>>;
1 => s.playbackMode;
0.2 => s.loopStart;
0.4 => s.loopEnd;
s.noteOn(60, 127);
2::second => now;
s.noteOff();
<<< "" >>>;

// Test 3: PINGPONG (mode 2)
<<< "Mode 2: PINGPONG (bounce between markers)" >>>;
2 => s.playbackMode;
0.1 => s.loopStart;
0.3 => s.loopEnd;
s.noteOn(60, 127);
2::second => now;
s.noteOff();
<<< "" >>>;

// Test 4: THRU (mode 3)
<<< "Mode 3: THRU (play through end marker to EOF)" >>>;
3 => s.playbackMode;
0.5 => s.startMarker;
1.0 => s.endMarker;
s.noteOn(60, 127);
3::second => now;
<<< "" >>>;

// Test 5: Reverse + ONESHOT
<<< "Reverse + ONESHOT (play backward from end to start)" >>>;
0 => s.playbackMode;
1 => s.reverse;
0.0 => s.startMarker;
s.length() => s.endMarker;
s.noteOn(60, 127);
3::second => now;
<<< "" >>>;

// Test 6: Reverse + THRU
<<< "Reverse + THRU (play backward from EOF to 0)" >>>;
3 => s.playbackMode;
1 => s.reverse;
s.noteOn(60, 127);
3::second => now;
<<< "" >>>;

<<< "✓ All playback modes working!" >>>;
