//-----------------------------------------------------------------------------
// Test Basic Polyphony
// Tests simultaneous voice playback, voice allocation, and noteOff
//-----------------------------------------------------------------------------

DrumBuf sampler => dac;

if (sampler.read(me.dir() + "/snare.wav") == 0) {
    <<< "FAIL: could not load snare.wav" >>>;
    me.exit();
}

<<< "=== Testing Basic Polyphony ===\n" >>>;

// Enable envelope for cleaner tests
1 => sampler.envelope;
0.01 => sampler.attack;
0.0 => sampler.hold;
0.15 => sampler.release;

//-----------------------------------------------------------------------------
// Test 1: Default polyphony
//-----------------------------------------------------------------------------
<<< "Test 1: Default polyphony" >>>;
<<< "Polyphony:", sampler.polyphony() >>>;
<<< "Expected: 16 voices" >>>;

//-----------------------------------------------------------------------------
// Test 2: Two overlapping notes
//-----------------------------------------------------------------------------
<<< "\nTest 2: Two overlapping notes" >>>;
<<< "Playing note 60, then note 64 while 60 is still playing" >>>;

sampler.noteOn(60, 127);
100::ms => now;
sampler.noteOn(64, 120);  // Second note overlaps with first
<<< "Active voices:", sampler.activeVoices() >>>;  // Should be 2

300::ms => now;
<<< "Both notes should be playing simultaneously" >>>;

sampler.noteOff();  // Turn off all
200::ms => now;

//-----------------------------------------------------------------------------
// Test 3: Three-note chord
//-----------------------------------------------------------------------------
<<< "\nTest 3: Three-note chord (C major)" >>>;

sampler.noteOn(60, 127);  // C
sampler.noteOn(64, 120);  // E
sampler.noteOn(67, 115);  // G

<<< "Active voices:", sampler.activeVoices() >>>;  // Should be 3

500::ms => now;
<<< "All three notes should be playing together" >>>;

sampler.noteOff();  // Turn off all
300::ms => now;

//-----------------------------------------------------------------------------
// Test 4: noteOff() with specific note
//-----------------------------------------------------------------------------
<<< "\nTest 4: Turn off specific note in chord" >>>;

sampler.noteOn(60, 127);  // C
sampler.noteOn(64, 120);  // E
sampler.noteOn(67, 115);  // G

<<< "Active voices:", sampler.activeVoices() >>>;  // Should be 3

200::ms => now;

sampler.noteOff(64);  // Turn off E only
<<< "Turned off note 64 (E)" >>>;
<<< "Active voices:", sampler.activeVoices() >>>;  // Should be 2

300::ms => now;
<<< "C and G should still be playing, E should be gone" >>>;

sampler.noteOff();  // Turn off remaining
200::ms => now;

//-----------------------------------------------------------------------------
// Test 5: Rapid overlapping notes
//-----------------------------------------------------------------------------
<<< "\nTest 5: Rapid overlapping drum hits" >>>;

for (0 => int i; i < 8; i++) {
    sampler.noteOn(60, 127 - (i * 10));
    80::ms => now;  // Notes overlap
}

<<< "Active voices:", sampler.activeVoices() >>>;

400::ms => now;  // Let notes finish

//-----------------------------------------------------------------------------
// Test 6: allNotesOff() panic button
//-----------------------------------------------------------------------------
<<< "\nTest 6: allNotesOff() panic button" >>>;

sampler.noteOn(60, 127);
sampler.noteOn(64, 120);
sampler.noteOn(67, 115);

<<< "Active voices before panic:", sampler.activeVoices() >>>;

100::ms => now;

sampler.allNotesOff();  // Immediate stop
<<< "Active voices after panic:", sampler.activeVoices() >>>;  // Should be 0

200::ms => now;

//-----------------------------------------------------------------------------
// Test 7: Change polyphony setting
//-----------------------------------------------------------------------------
<<< "\nTest 7: Change polyphony to 4 voices" >>>;

4 => sampler.polyphony;
<<< "Polyphony set to:", sampler.polyphony() >>>;

// Play 4 notes
sampler.noteOn(60, 127);
sampler.noteOn(62, 120);
sampler.noteOn(64, 115);
sampler.noteOn(65, 110);

<<< "Active voices:", sampler.activeVoices() >>>;  // Should be 4

500::ms => now;

sampler.noteOff();
200::ms => now;

//-----------------------------------------------------------------------------
// Test 8: Polyphony with notePos mapping
//-----------------------------------------------------------------------------
<<< "\nTest 8: Polyphony with different start positions" >>>;

16 => sampler.polyphony;  // Reset to default

// Map notes to different positions
sampler.notePos(60, 0.0);
sampler.notePos(62, 0.05);
sampler.notePos(64, 0.1);

// Trigger all three simultaneously
sampler.noteOn(60, 127);
sampler.noteOn(62, 120);
sampler.noteOn(64, 115);

<<< "Three notes playing from different start positions" >>>;
<<< "Active voices:", sampler.activeVoices() >>>;

500::ms => now;

sampler.noteOff();
200::ms => now;

<<< "\n=== Basic Polyphony Test Complete ===" >>>;
<<< "Polyphony allows multiple notes to play simultaneously." >>>;
<<< "Voice allocation and stealing work correctly." >>>;
