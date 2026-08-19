//-----------------------------------------------------------------------------
// DrumBuf Note Tracking Test
// Tests TRADITIONAL vs GRANULAR note tracking modes
//-----------------------------------------------------------------------------

DrumBuf sampler => dac;

if (sampler.read(me.dir() + "/snare.wav") == 0) {
    <<< "FAIL: could not load snare.wav" >>>;
    me.exit();
}

<<< "=== DrumBuf Note Tracking Test ===" >>>;
<<< "" >>>;
<<< "Note tracking modes:" >>>;
<<< "  OFF mode (0, default): note number doesn't affect pitch" >>>;
<<< "  TRADITIONAL mode (1): note affects pitch AND duration (like tape speed)" >>>;
<<< "  GRANULAR mode (2): note affects pitch ONLY (duration stays constant)" >>>;
<<< "  THRU playback mode: note tracking is automatically disabled" >>>;
<<< "" >>>;

// Set root note to C4 (60)
60 => sampler.rootNote;

//-----------------------------------------------------------------------------
// Test 1: OFF mode (default) - Rising scale
//-----------------------------------------------------------------------------
<<< "Test 1: OFF mode (default) - Rising scale (C4, D4, E4, F4, G4)" >>>;
<<< "All notes should sound the same pitch (only position changes)" >>>;
0 => sampler.noteTracking;  // OFF
<<< "  noteTracking:", sampler.noteTracking() >>>;
<<< "  rootNote:", sampler.rootNote() >>>;

[60, 62, 64, 65, 67] @=> int scale[];  // C, D, E, F, G
for (0 => int i; i < scale.size(); i++) {
    sampler.noteOn(scale[i], 127);
    400::ms => now;
    sampler.noteOff();
    100::ms => now;
}
1::second => now;

//-----------------------------------------------------------------------------
// Test 2: TRADITIONAL mode - Rising scale
//-----------------------------------------------------------------------------
<<< "Test 2: TRADITIONAL mode - Rising scale (C4, D4, E4, F4, G4)" >>>;
<<< "Each note should be higher pitch AND shorter duration" >>>;
1 => sampler.noteTracking;  // TRADITIONAL
<<< "  noteTracking:", sampler.noteTracking() >>>;

for (0 => int i; i < scale.size(); i++) {
    sampler.noteOn(scale[i], 127);
    400::ms => now;
    sampler.noteOff();
    100::ms => now;
}
1::second => now;

//-----------------------------------------------------------------------------
// Test 3: GRANULAR mode - Rising scale (same duration)
//-----------------------------------------------------------------------------
<<< "Test 3: GRANULAR mode - Rising scale (C4, D4, E4, F4, G4)" >>>;
<<< "Each note should be higher pitch but SAME duration" >>>;
2 => sampler.noteTracking;  // GRANULAR
<<< "  noteTracking:", sampler.noteTracking() >>>;

for (0 => int i; i < scale.size(); i++) {
    sampler.noteOn(scale[i], 127);
    400::ms => now;
    sampler.noteOff();
    100::ms => now;
}
1::second => now;

//-----------------------------------------------------------------------------
// Test 4: TRADITIONAL mode - Full chromatic scale
//-----------------------------------------------------------------------------
<<< "Test 4: TRADITIONAL mode - Chromatic scale (1 octave)" >>>;
1 => sampler.noteTracking;  // TRADITIONAL

for (60 => int note; note <= 72; note++) {
    sampler.noteOn(note, 120);
    200::ms => now;
    sampler.noteOff();
    50::ms => now;
}
1::second => now;

//-----------------------------------------------------------------------------
// Test 5: GRANULAR mode - Full chromatic scale
//-----------------------------------------------------------------------------
<<< "Test 5: GRANULAR mode - Chromatic scale (1 octave)" >>>;
2 => sampler.noteTracking;  // GRANULAR

for (60 => int note; note <= 72; note++) {
    sampler.noteOn(note, 120);
    200::ms => now;
    sampler.noteOff();
    50::ms => now;
}
1::second => now;

//-----------------------------------------------------------------------------
// Test 6: THRU mode - Note tracking auto-disabled
//-----------------------------------------------------------------------------
<<< "Test 6: THRU mode - Note tracking automatically disabled" >>>;
<<< "All notes should sound the same (tracking is OFF in THRU mode)" >>>;
1 => sampler.noteTracking;  // TRADITIONAL (but will be ignored in THRU)
3 => sampler.playbackMode;  // THRU mode

for (0 => int i; i < 4; i++) {
    sampler.noteOn(60 + i * 3, 127);  // Play C4, Eb4, Gb4, A4
    400::ms => now;
    sampler.noteOff();
    100::ms => now;
}

0 => sampler.playbackMode;  // Reset to ONESHOT
1::second => now;

//-----------------------------------------------------------------------------
// Test 7: Different root notes
//-----------------------------------------------------------------------------
<<< "Test 7: Changing root note" >>>;
<<< "Playing G4 (67) with different root notes in TRADITIONAL mode" >>>;
1 => sampler.noteTracking;  // TRADITIONAL

<<< "  Root = C4 (60): G4 should be +7 semitones" >>>;
60 => sampler.rootNote;
sampler.noteOn(67, 127);
600::ms => now;
sampler.noteOff();
400::ms => now;

<<< "  Root = G4 (67): G4 should be at original pitch" >>>;
67 => sampler.rootNote;
sampler.noteOn(67, 127);
600::ms => now;
sampler.noteOff();
400::ms => now;

<<< "  Root = E4 (64): G4 should be +3 semitones" >>>;
64 => sampler.rootNote;
sampler.noteOn(67, 127);
600::ms => now;
sampler.noteOff();
1::second => now;

//-----------------------------------------------------------------------------
// Test 8: Note tracking combined with transpose
//-----------------------------------------------------------------------------
<<< "Test 8: Note tracking + manual transpose" >>>;
<<< "GRANULAR mode with +5 semitone transpose" >>>;
2 => sampler.noteTracking;  // GRANULAR
60 => sampler.rootNote;
5.0 => sampler.transpose;  // +5 semitones

for (0 => int i; i < scale.size(); i++) {
    sampler.noteOn(scale[i], 127);
    400::ms => now;
    sampler.noteOff();
    100::ms => now;
}

0.0 => sampler.transpose;  // Reset
1::second => now;

//-----------------------------------------------------------------------------
// Test 9: TRADITIONAL vs GRANULAR comparison
//-----------------------------------------------------------------------------
<<< "Test 9: Direct comparison - Same note played in both modes" >>>;

<<< "  TRADITIONAL: Note C5 (72, +12 semitones from root)" >>>;
1 => sampler.noteTracking;
60 => sampler.rootNote;
sampler.noteOn(72, 127);
800::ms => now;
sampler.noteOff();
500::ms => now;

<<< "  GRANULAR: Note C5 (72, +12 semitones from root)" >>>;
2 => sampler.noteTracking;
sampler.noteOn(72, 127);
800::ms => now;
sampler.noteOff();

<<< "" >>>;
<<< "=== Note Tracking Test Complete ===" >>>;
<<< "" >>>;
<<< "Summary:" >>>;
<<< "  0 = OFF: Note number doesn't affect pitch (default)" >>>;
<<< "  1 = TRADITIONAL: Note affects pitch+time (tape speed)" >>>;
<<< "  2 = GRANULAR: Note affects pitch only (constant duration)" >>>;
<<< "  Auto-disabled in THRU mode for natural drum ring-out" >>>;
<<< "" >>>;
<<< "TRADITIONAL mode: Higher notes = higher pitch + shorter duration" >>>;
<<< "GRANULAR mode: Higher notes = higher pitch, same duration" >>>;
<<< "OFF mode: All notes play at same pitch (best for drum slicing)" >>>;
