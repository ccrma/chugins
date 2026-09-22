//-----------------------------------------------------------------------------
// Test THRU Mode Monophonic Behavior
// Tests that THRU mode stops all active voices on new noteOn (drum choking)
//-----------------------------------------------------------------------------

DrumBuf sampler => dac;

if (sampler.read(me.dir() + "/snare.wav") == 0) {
    <<< "FAIL: could not load snare.wav" >>>;
    me.exit();
}

<<< "=== Testing THRU Mode Monophonic Behavior ===\n" >>>;

//-----------------------------------------------------------------------------
// Test 1: ONESHOT mode is polyphonic (baseline)
//-----------------------------------------------------------------------------
<<< "Test 1: ONESHOT mode - polyphonic (baseline)" >>>;
0 => sampler.playbackMode;  // ONESHOT

sampler.noteOn(60, 127);
<<< "After first note - Active:", sampler.activeVoices() >>>;

100::ms => now;

sampler.noteOn(62, 120);
<<< "After second note - Active:", sampler.activeVoices(), "(expect: 2)" >>>;
<<< "Both notes should be playing" >>>;

300::ms => now;
sampler.allNotesOff();
100::ms => now;

//-----------------------------------------------------------------------------
// Test 2: THRU mode is monophonic (choking)
//-----------------------------------------------------------------------------
<<< "\nTest 2: THRU mode - monophonic (drum choking)" >>>;
3 => sampler.playbackMode;  // THRU

sampler.noteOn(60, 127);
<<< "After first note - Active:", sampler.activeVoices() >>>;

200::ms => now;
<<< "First note still active:", sampler.activeVoices() >>>;

sampler.noteOn(62, 120);  // This should STOP the first note
<<< "After second note - Active:", sampler.activeVoices(), "(expect: 1)" >>>;
<<< "First note should be chopped, only second note playing" >>>;

300::ms => now;

sampler.noteOn(64, 115);  // This should STOP the second note
<<< "After third note - Active:", sampler.activeVoices(), "(expect: 1)" >>>;
<<< "Second note should be chopped, only third note playing" >>>;

500::ms => now;

//-----------------------------------------------------------------------------
// Test 3: Hi-hat choking simulation
//-----------------------------------------------------------------------------
<<< "\nTest 3: Hi-hat open/closed choking (THRU mode)" >>>;
<<< "Simulates realistic hi-hat behavior" >>>;

// Map note 60 to start (closed hi-hat sound)
// Map note 62 to middle (open hi-hat sound - longer tail)
sampler.notePos(60, 0.0);
sampler.notePos(62, 0.05);

// Closed hit
sampler.noteOn(60, 127);
<<< "Closed hi-hat" >>>;
100::ms => now;

// Open hit (chokes closed)
sampler.noteOn(62, 120);
<<< "Open hi-hat (chopped closed)" >>>;
200::ms => now;

// Another closed (chokes open - important!)
sampler.noteOn(60, 110);
<<< "Closed hi-hat (chopped open)" >>>;
150::ms => now;

// Another open
sampler.noteOn(62, 100);
<<< "Open hi-hat again" >>>;
300::ms => now;

//-----------------------------------------------------------------------------
// Test 4: Rapid drum hits with choking
//-----------------------------------------------------------------------------
<<< "\nTest 4: Rapid drum hits with THRU mode" >>>;
<<< "Each hit chokes the previous one" >>>;

for (0 => int i; i < 8; i++) {
    sampler.noteOn(60, 120 - (i * 10));
    <<< "Hit", i + 1, "- Active:", sampler.activeVoices(), "(should always be 1)" >>>;
    100::ms => now;
}

500::ms => now;

//-----------------------------------------------------------------------------
// Test 5: Compare THRU vs ONESHOT with rapid hits
//-----------------------------------------------------------------------------
<<< "\nTest 5: Compare THRU vs ONESHOT modes" >>>;

<<< "\n5a: ONESHOT mode - notes pile up (polyphonic)" >>>;
0 => sampler.playbackMode;

sampler.noteOn(60, 127);
50::ms => now;
sampler.noteOn(60, 120);
50::ms => now;
sampler.noteOn(60, 115);
<<< "Active voices:", sampler.activeVoices(), "(expect: 3)" >>>;

300::ms => now;
sampler.allNotesOff();
100::ms => now;

<<< "\n5b: THRU mode - each hit chokes previous (monophonic)" >>>;
3 => sampler.playbackMode;

sampler.noteOn(60, 127);
50::ms => now;
<<< "After first hit - Active:", sampler.activeVoices() >>>;

sampler.noteOn(60, 120);  // Chokes first
50::ms => now;
<<< "After second hit - Active:", sampler.activeVoices(), "(expect: 1)" >>>;

sampler.noteOn(60, 115);  // Chokes second
<<< "After third hit - Active:", sampler.activeVoices(), "(expect: 1)" >>>;

300::ms => now;

//-----------------------------------------------------------------------------
// Test 6: Switch between modes
//-----------------------------------------------------------------------------
<<< "\nTest 6: Switch from THRU to ONESHOT mid-session" >>>;

3 => sampler.playbackMode;  // Start with THRU
sampler.noteOn(60, 127);
<<< "THRU mode - Active:", sampler.activeVoices() >>>;
100::ms => now;

sampler.noteOn(62, 120);  // Chokes
<<< "After choke - Active:", sampler.activeVoices(), "(expect: 1)" >>>;
100::ms => now;

// Switch to ONESHOT (polyphonic)
0 => sampler.playbackMode;
sampler.noteOn(64, 115);  // Should NOT choke
<<< "After switch to ONESHOT - Active:", sampler.activeVoices(), "(expect: 2)" >>>;

300::ms => now;

sampler.allNotesOff();

<<< "\n=== THRU Mode Monophonic Test Complete ===" >>>;
<<< "THRU mode forces monophonic behavior for realistic drum choking." >>>;
<<< "Other modes (ONESHOT, LOOP, PINGPONG) remain polyphonic." >>>;
