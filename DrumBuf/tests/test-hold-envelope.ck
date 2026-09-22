//-----------------------------------------------------------------------------
// Test Hold Envelope
// Tests minimum hold time feature (prevents early release)
//-----------------------------------------------------------------------------

DrumBuf sampler => dac;

if (sampler.read(me.dir() + "/snare.wav") == 0) {
    <<< "FAIL: could not load snare.wav" >>>;
    me.exit();
}

<<< "=== Testing Hold Envelope (Minimum Note Duration) ===\n" >>>;

// Enable envelope
1 => sampler.envelope;
0.01 => sampler.attack;    // Fast attack
0.1 => sampler.release;    // Normal release

//-----------------------------------------------------------------------------
// Test 1: No hold time (default - immediate release on noteOff)
//-----------------------------------------------------------------------------
<<< "Test 1: No hold time (0.0s, default)" >>>;
<<< "noteOff after 100ms should start release immediately" >>>;
0.0 => sampler.hold;

sampler.noteOn(60, 127);
100::ms => now;
sampler.noteOff();  // Release starts immediately
200::ms => now;     // Wait for release

//-----------------------------------------------------------------------------
// Test 2: Short hold time (50ms)
//-----------------------------------------------------------------------------
<<< "\nTest 2: Short hold (0.05s), noteOff after 100ms" >>>;
<<< "Since 100ms > 50ms, release should start immediately" >>>;
0.05 => sampler.hold;

sampler.noteOn(60, 127);
100::ms => now;
sampler.noteOff();  // 100ms elapsed, hold time met, release starts
150::ms => now;     // Wait for release

//-----------------------------------------------------------------------------
// Test 3: Hold time longer than note (delayed release)
//-----------------------------------------------------------------------------
<<< "\nTest 3: Hold time (0.3s), noteOff after 100ms" >>>;
<<< "Early noteOff! Release should be delayed until 300ms total" >>>;
0.3 => sampler.hold;

sampler.noteOn(60, 127);
100::ms => now;
sampler.noteOff();  // Too early! Only 100ms elapsed, need 300ms
// Note continues until hold time met
300::ms => now;     // Wait for hold + release

//-----------------------------------------------------------------------------
// Test 4: Very long hold time
//-----------------------------------------------------------------------------
<<< "\nTest 4: Long hold time (0.5s), noteOff after 50ms" >>>;
<<< "Very early noteOff! Note forced to last at least 500ms" >>>;
0.5 => sampler.hold;

sampler.noteOn(60, 127);
50::ms => now;
sampler.noteOff();  // Extremely early, but note must last 500ms
600::ms => now;     // Wait for hold + release

//-----------------------------------------------------------------------------
// Test 5: noteOff after hold time already met
//-----------------------------------------------------------------------------
<<< "\nTest 5: Hold time (0.2s), noteOff after 300ms" >>>;
<<< "Late noteOff (after hold met) should release immediately" >>>;
0.2 => sampler.hold;

sampler.noteOn(60, 127);
300::ms => now;     // Wait longer than hold time
sampler.noteOff();  // Hold already met, release starts immediately
200::ms => now;     // Wait for release

//-----------------------------------------------------------------------------
// Test 6: Multiple rapid notes with hold
//-----------------------------------------------------------------------------
<<< "\nTest 6: Rapid notes with 200ms hold time" >>>;
<<< "Testing retriggering behavior" >>>;
0.2 => sampler.hold;

for (0 => int i; i < 4; i++) {
    sampler.noteOn(60, 127 - (i * 20));
    80::ms => now;   // Each note only 80ms, but hold forces 200ms
    sampler.noteOff();
    50::ms => now;   // Small gap between notes
}
300::ms => now;  // Wait for last note to finish

//-----------------------------------------------------------------------------
// Test 7: Compare hold=0 vs hold=0.3 on same rhythm
//-----------------------------------------------------------------------------
<<< "\nTest 7a: Staccato rhythm with no hold (0.0s)" >>>;
0.0 => sampler.hold;

for (0 => int i; i < 4; i++) {
    sampler.noteOn(60, 127);
    50::ms => now;   // Short notes
    sampler.noteOff();
    100::ms => now;  // Gaps between notes
}

<<< "Test 7b: Same rhythm with hold (0.3s)" >>>;
<<< "Should sound more legato - notes sustained longer" >>>;
0.3 => sampler.hold;

for (0 => int i; i < 4; i++) {
    sampler.noteOn(60, 127);
    50::ms => now;   // Short notes
    sampler.noteOff();
    100::ms => now;  // Gaps, but notes ring longer
}
300::ms => now;  // Wait for last note

//-----------------------------------------------------------------------------
// Test 8: Hold with instant attack and release
//-----------------------------------------------------------------------------
<<< "\nTest 8: Hold (0.2s) with instant attack (0.0s) and instant release (0.0s)" >>>;
<<< "Note lasts exactly 200ms then cuts off instantly" >>>;
0.0 => sampler.attack;
0.2 => sampler.hold;
0.0 => sampler.release;

sampler.noteOn(60, 127);
50::ms => now;
sampler.noteOff();  // Early, but held for 200ms total
200::ms => now;

<<< "\n=== Hold Envelope Test Complete ===" >>>;
<<< "Hold time ensures minimum note duration." >>>;
<<< "Early noteOff() calls are delayed until hold time elapses." >>>;
<<< "This is useful for preventing overly short drum hits." >>>;
