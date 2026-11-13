//-----------------------------------------------------------------------------
// Test Polyphony Verification
// Simple test to verify polyphony is actually working
// Uses LOOP mode to keep voices playing
//-----------------------------------------------------------------------------

DrumBuf sampler => dac;

if (sampler.read(me.dir() + "/snare.wav") == 0) {
    <<< "FAIL: could not load snare.wav" >>>;
    me.exit();
}

<<< "=== Polyphony Verification Test ===\n" >>>;

// Use LOOP mode so voices don't finish
1 => sampler.playbackMode;  // LOOP
0.1 => sampler.loopStart;
0.9 => sampler.loopEnd;

// Enable envelope so noteOff works (but with long release)
1 => sampler.envelope;
0.01 => sampler.attack;
0.0 => sampler.hold;
1.0 => sampler.release;  // Long release so we can observe voices

//-----------------------------------------------------------------------------
// Test: Play multiple notes and verify they all stay active
//-----------------------------------------------------------------------------
<<< "Playing 5 notes with LOOP mode (no envelope)" >>>;
<<< "All notes should stay active until we turn them off\n" >>>;

<<< "Initial active voices:", sampler.activeVoices() >>>;

sampler.noteOn(60, 127);
100::ms => now;
<<< "After note 60 - Active:", sampler.activeVoices(), "(expect: 1)" >>>;

sampler.noteOn(62, 120);
100::ms => now;
<<< "After note 62 - Active:", sampler.activeVoices(), "(expect: 2)" >>>;

sampler.noteOn(64, 115);
100::ms => now;
<<< "After note 64 - Active:", sampler.activeVoices(), "(expect: 3)" >>>;

sampler.noteOn(65, 110);
100::ms => now;
<<< "After note 65 - Active:", sampler.activeVoices(), "(expect: 4)" >>>;

sampler.noteOn(67, 105);
100::ms => now;
<<< "After note 67 - Active:", sampler.activeVoices(), "(expect: 5)" >>>;

<<< "\nAll 5 notes should be playing simultaneously (looping)" >>>;

500::ms => now;

<<< "\nTurning off note 64..." >>>;
sampler.noteOff(64);
<<< "Active voices (immediately after noteOff):", sampler.activeVoices(), "(still 5 - releasing)" >>>;
1200::ms => now;  // Wait for 1.0s release to complete
<<< "Active voices (after release):", sampler.activeVoices(), "(expect: 4)" >>>;

<<< "\nTurning off note 62..." >>>;
sampler.noteOff(62);
1200::ms => now;  // Wait for release
<<< "Active voices (after release):", sampler.activeVoices(), "(expect: 3)" >>>;

<<< "\nUsing allNotesOff() panic button (immediate stop)..." >>>;
sampler.allNotesOff();
<<< "Active voices (immediate):", sampler.activeVoices(), "(expect: 0)" >>>;

<<< "\n=== Polyphony Verification Complete ===" >>>;
<<< "If counts match expectations, polyphony is working correctly!" >>>;
