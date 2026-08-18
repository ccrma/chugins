//-----------------------------------------------------------------------------
// Test Start/End Markers
//-----------------------------------------------------------------------------

DrumBuf sampler => dac;

if (sampler.read(me.dir() + "/SovereignBreak.wav") == 0) {
    <<< "FAIL: could not load SovereignBreak.wav" >>>;
    me.exit();
}

<<< "=== Testing Start/End Markers ===" >>>;
<<< "" >>>;

// Test 1: Default markers (full file)
<<< "Test 1: Full file playback (default markers)" >>>;
<<< "Start marker:", sampler.startMarker(), "seconds" >>>;
<<< "End marker:", sampler.endMarker(), "seconds" >>>;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
1::second => now;

// Test 2: Play only first half (set end marker to half duration)
<<< "Test 2: First half only (end marker = 0.5 seconds)" >>>;
0.5 => sampler.endMarker;
<<< "Start marker:", sampler.startMarker(), "seconds" >>>;
<<< "End marker:", sampler.endMarker(), "seconds" >>>;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
1::second => now;

// Test 3: Play only second half (set start marker to half duration)
<<< "Test 3: Second half only (start marker = 0.5 seconds)" >>>;
0.5 => sampler.startMarker;
1.0 => sampler.endMarker;  // Reset to full duration
<<< "Start marker:", sampler.startMarker(), "seconds" >>>;
<<< "End marker:", sampler.endMarker(), "seconds" >>>;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
1::second => now;

// Test 4: Play middle section only
<<< "Test 4: Middle section (0.3 - 0.7 seconds)" >>>;
0.3 => sampler.startMarker;
0.7 => sampler.endMarker;
<<< "Start marker:", sampler.startMarker(), "seconds" >>>;
<<< "End marker:", sampler.endMarker(), "seconds" >>>;
sampler.noteOn(60, 127);
2::second => now;
sampler.noteOff();
1::second => now;

// Test 5: Short segment with looping
<<< "Test 5: Short looped segment (0.1 - 0.3 seconds, looping)" >>>;
0.1 => sampler.startMarker;
0.3 => sampler.endMarker;
1 => sampler.playbackMode;  // LOOP
<<< "Start marker:", sampler.startMarker(), "seconds" >>>;
<<< "End marker:", sampler.endMarker(), "seconds" >>>;
<<< "Playback mode: LOOP" >>>;
sampler.noteOn(60, 127);
3::second => now;
sampler.noteOff();
0 => sampler.playbackMode;  // Reset to ONESHOT
1::second => now;

// Test 6: Note positions in seconds (clamped to marker range)
<<< "Test 6: Note positions in seconds (clamped to marker range)" >>>;
0.0 => sampler.startMarker;
1.0 => sampler.endMarker;
sampler.notePos(60, 0.0);   // C4 starts at 0.0 seconds
sampler.notePos(62, 0.5);   // D4 starts at 0.5 seconds
sampler.notePos(64, 1.0);   // E4 starts at 1.0 seconds
<<< "Start marker:", sampler.startMarker(), "seconds" >>>;
<<< "End marker:", sampler.endMarker(), "seconds" >>>;

<<< "  Playing C4 (note pos = 0.0s)" >>>;
sampler.noteOn(60, 127);
600::ms => now;
sampler.noteOff();
400::ms => now;

<<< "  Playing D4 (note pos = 0.5s)" >>>;
sampler.noteOn(62, 127);
600::ms => now;
sampler.noteOff();
400::ms => now;

<<< "  Playing E4 (note pos = 1.0s)" >>>;
sampler.noteOn(64, 127);
600::ms => now;
sampler.noteOff();
400::ms => now;

// Test 7: Very short segment for granular effect
<<< "Test 7: Tiny segment with time stretch (0.2 - 0.25 seconds, rate 0.25x)" >>>;
0.2 => sampler.startMarker;
0.25 => sampler.endMarker;
0.25 => sampler.rate;
1 => sampler.playbackMode;  // LOOP
<<< "Start marker:", sampler.startMarker(), "seconds" >>>;
<<< "End marker:", sampler.endMarker(), "seconds" >>>;
<<< "Rate: 0.25x (slow)" >>>;
sampler.noteOn(60, 127);
4::second => now;
sampler.noteOff();

<<< "" >>>;
<<< "Start/End marker test complete" >>>;
<<< "" >>>;
<<< "Note: Start/end markers are in seconds and sample-rate independent." >>>;
<<< "Note positions (set via notePos) are also in seconds and are clamped to marker range during playback." >>>;
