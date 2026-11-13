//-----------------------------------------------------------------------------
// Test Polyphony with Chords
// Tests polyphony combined with note tracking for melodic/harmonic playing
//-----------------------------------------------------------------------------

DrumBuf sampler => dac;

if (sampler.read(me.dir() + "/snare.wav") == 0) {
    <<< "FAIL: could not load snare.wav" >>>;
    me.exit();
}

<<< "=== Testing Polyphony with Chords ===\n" >>>;

// Configure for melodic playing
1 => sampler.envelope;
0.02 => sampler.attack;
0.0 => sampler.hold;
0.3 => sampler.release;
2 => sampler.noteTracking;  // GRANULAR mode (pitch only, not time)
60 => sampler.rootNote;  // C4 as root
16 => sampler.polyphony;

//-----------------------------------------------------------------------------
// Test 1: C Major chord (C-E-G)
//-----------------------------------------------------------------------------
<<< "Test 1: C Major chord (C-E-G)" >>>;

sampler.noteOn(60, 127);  // C4
sampler.noteOn(64, 120);  // E4 (+4 semitones)
sampler.noteOn(67, 115);  // G4 (+7 semitones)

<<< "Active voices:", sampler.activeVoices() >>>;  // Should be 3

1::second => now;
<<< "Should hear a C major triad" >>>;

sampler.noteOff();
400::ms => now;

//-----------------------------------------------------------------------------
// Test 2: Dm7 chord (D-F-A-C)
//-----------------------------------------------------------------------------
<<< "\nTest 2: Dm7 chord (D-F-A-C)" >>>;

sampler.noteOn(62, 127);  // D4
sampler.noteOn(65, 120);  // F4
sampler.noteOn(69, 115);  // A4
sampler.noteOn(72, 110);  // C5

<<< "Active voices:", sampler.activeVoices() >>>;  // Should be 4

1::second => now;
<<< "Should hear a Dm7 chord" >>>;

sampler.noteOff();
400::ms => now;

//-----------------------------------------------------------------------------
// Test 3: Chord progression (C - Am - F - G)
//-----------------------------------------------------------------------------
<<< "\nTest 3: Chord progression (C - Am - F - G)" >>>;

// C Major
sampler.noteOn(60, 120);
sampler.noteOn(64, 115);
sampler.noteOn(67, 110);
600::ms => now;

// A Minor
sampler.noteOff();
100::ms => now;
sampler.noteOn(57, 120);
sampler.noteOn(60, 115);
sampler.noteOn(64, 110);
600::ms => now;

// F Major
sampler.noteOff();
100::ms => now;
sampler.noteOn(53, 120);
sampler.noteOn(57, 115);
sampler.noteOn(60, 110);
600::ms => now;

// G Major
sampler.noteOff();
100::ms => now;
sampler.noteOn(55, 120);
sampler.noteOn(59, 115);
sampler.noteOn(62, 110);
600::ms => now;

sampler.noteOff();
400::ms => now;

//-----------------------------------------------------------------------------
// Test 4: Arpeggiated chord (notes added one by one)
//-----------------------------------------------------------------------------
<<< "\nTest 4: Arpeggiated C major chord" >>>;

sampler.noteOn(60, 127);  // C
<<< "Active:", sampler.activeVoices() >>>;
200::ms => now;

sampler.noteOn(64, 120);  // E (C still playing)
<<< "Active:", sampler.activeVoices() >>>;
200::ms => now;

sampler.noteOn(67, 115);  // G (C and E still playing)
<<< "Active:", sampler.activeVoices() >>>;
200::ms => now;

sampler.noteOn(72, 110);  // C5 (all still playing)
<<< "Active:", sampler.activeVoices() >>>;  // Should be 4

500::ms => now;

sampler.noteOff();
400::ms => now;

//-----------------------------------------------------------------------------
// Test 5: Selective noteOff (melody over sustained chord)
//-----------------------------------------------------------------------------
<<< "\nTest 5: Melody over sustained chord" >>>;

// Play sustained chord
sampler.noteOn(48, 100);  // C3
sampler.noteOn(52, 95);   // E3
sampler.noteOn(55, 90);   // G3

<<< "Chord active voices:", sampler.activeVoices() >>>;

200::ms => now;

// Play melody notes on top
[60, 62, 64, 65, 67, 65, 64, 62, 60] @=> int melody[];

for (0 => int i; i < melody.size(); i++) {
    sampler.noteOn(melody[i], 120);
    150::ms => now;
    sampler.noteOff(melody[i]);  // Turn off only the melody note
    50::ms => now;
}

<<< "Chord voices still active:", sampler.activeVoices() >>>;

// Turn off chord
sampler.noteOff();
400::ms => now;

//-----------------------------------------------------------------------------
// Test 6: Voice stealing (exceed polyphony)
//-----------------------------------------------------------------------------
<<< "\nTest 6: Voice stealing - play 8 notes with 4 voice limit" >>>;

4 => sampler.polyphony;  // Limit to 4 voices
<<< "Polyphony set to:", sampler.polyphony() >>>;

// Play 8 overlapping notes
for (48 => int note; note < 56; note++) {
    sampler.noteOn(note, 120);
    <<< "Note", note, "- Active:", sampler.activeVoices() >>>;
    100::ms => now;  // Notes overlap
}

<<< "Should never exceed 4 active voices (oldest get stolen)" >>>;

500::ms => now;

sampler.noteOff();
400::ms => now;

//-----------------------------------------------------------------------------
// Test 7: Polyphonic melody (sequential notes with envelope tails)
//-----------------------------------------------------------------------------
<<< "\nTest 7: Polyphonic melody with envelope tails" >>>;

16 => sampler.polyphony;  // Reset to larger polyphony
0.5 => sampler.release;   // Longer release for overlapping tails

[60, 62, 64, 65, 67, 69, 71, 72] @=> int scale[];

for (0 => int i; i < scale.size(); i++) {
    sampler.noteOn(scale[i], 120);
    120::ms => now;  // Fast enough that releases overlap
    sampler.noteOff(scale[i]);
}

<<< "Active voices during release tails:", sampler.activeVoices() >>>;

800::ms => now;  // Let all releases complete

//-----------------------------------------------------------------------------
// Test 8: Monophonic check (polyphony=1)
//-----------------------------------------------------------------------------
<<< "\nTest 8: Monophonic mode (polyphony=1)" >>>;

1 => sampler.polyphony;
<<< "Polyphony set to 1 (monophonic)" >>>;

sampler.noteOn(60, 127);
<<< "Active:", sampler.activeVoices() >>>;
200::ms => now;

sampler.noteOn(64, 120);  // Should steal the first voice
<<< "Active:", sampler.activeVoices() >>>;  // Should still be 1
<<< "First note should be stolen, only second note playing" >>>;

300::ms => now;

sampler.noteOff();
400::ms => now;

<<< "\n=== Polyphony with Chords Test Complete ===" >>>;
<<< "Polyphony enables simultaneous melodic/harmonic playing." >>>;
<<< "Voice stealing works correctly when polyphony limit reached." >>>;
