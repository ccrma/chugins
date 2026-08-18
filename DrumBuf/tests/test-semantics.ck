//-----------------------------------------------------------------------------
// Automated checks of the sampler core's structural behaviour, using
// activeVoices() and time (no listening needed). Prints PASS/FAIL lines;
// run-tests.sh greps for FAIL. Every check is silent-safe: route to blackhole.
//-----------------------------------------------------------------------------

DrumBuf s => blackhole;
if (s.read(me.dir() + "/SovereignBreak.wav") == 0) {
    <<< "FAIL: could not load SovereignBreak.wav" >>>;
    me.exit();
}

0 => int failures;
fun void check(int cond, string what) {
    if (cond) <<< "PASS:", what >>>;
    else { <<< "FAIL:", what >>>; failures++; }
}
fun void reset() {
    s.allNotesOff();
    0 => s.playbackMode; 0 => s.reverse; 0 => s.envelope;
    0.0 => s.transpose; 1.0 => s.rate; 0.0 => s.decay; 0.0 => s.fadeOut;
    0.0 => s.startMarker; s.length() => s.endMarker;
    16 => s.polyphony;
}

s.length() => float dur;
// A three-slice map: note 0 = [0, 0.25 s), note 1 = [0.25, 0.5), note 2 = [0.5, end)
s.notePos(0, 0.0);
s.notePos(1, 0.25);
s.notePos(2, 0.5);

// --- 1. ONESHOT auto-stops at the next note position ------------------------
reset();
s.noteOn(1, 127);
100::ms => now;
check(s.activeVoices() == 1, "slice voice is active mid-slice");
200::ms => now;   // 0.3 s in: slice 1 is 0.25 s long
check(s.activeVoices() == 0, "ONESHOT voice stopped at the next note position");

// --- 2. reverse ONESHOT plays the note's OWN slice (same length), not the previous
reset();
1 => s.reverse;
s.noteOn(1, 127);
100::ms => now;
check(s.activeVoices() == 1, "reverse slice voice is active mid-slice");
200::ms => now;
check(s.activeVoices() == 0, "reverse ONESHOT voice stopped after its own slice length");
s.noteOn(0, 127);           // note position 0: previously died on the first tick
50::ms => now;
check(s.activeVoices() == 1, "reverse ONESHOT at position 0 plays (its slice, backwards)");

// --- 3. rate stretches the slice duration -----------------------------------
reset();
0.5 => s.rate;              // half speed: the 0.25 s slice lasts ~0.5 s
s.noteOn(1, 127);
350::ms => now;
check(s.activeVoices() == 1, "rate 0.5 keeps the slice voice alive past its normal length");
250::ms => now;
check(s.activeVoices() == 0, "...and it stops at ~2x the length");

// --- 4. noteOff stops looping voices even without an envelope ---------------
reset();
1 => s.playbackMode;        // LOOP
s.noteOn(60, 127);
100::ms => now;
check(s.activeVoices() == 1, "LOOP voice active");
s.noteOff(60);
1::samp => now;
check(s.activeVoices() == 0, "noteOff stops a LOOP voice with the envelope off");

// --- 5. envelope release: voice outlives noteOff by ~release, then ends ------
reset();
1 => s.playbackMode; 1 => s.envelope; 0.001 => s.attack; 0.05 => s.release;
s.noteOn(60, 127);
50::ms => now;
s.noteOff(60);
10::ms => now;
check(s.activeVoices() == 1, "voice still releasing right after noteOff");
200::ms => now;
check(s.activeVoices() == 0, "voice gone after the release time");

// --- 6. envelope enabled after a voice started without one: still releasable
reset();
1 => s.playbackMode;
s.noteOn(60, 127);
50::ms => now;
1 => s.envelope; 0.01 => s.release;
s.noteOff(60);
200::ms => now;
check(s.activeVoices() == 0, "voice started without envelope is released after enabling it");

// --- 7. shrinking polyphony stops the voices above the new limit ------------
reset();
1 => s.playbackMode; 8 => s.polyphony;
for (0 => int i; i < 8; i++) s.noteOn(60 + i, 100);
1::samp => now;
check(s.activeVoices() == 8, "8 voices at polyphony 8");
4 => s.polyphony;
check(s.activeVoices() == 4, "polyphony 4 leaves 4 active");
8 => s.polyphony;
check(s.activeVoices() == 4, "growing polyphony again does not resurrect stopped voices");

// --- 8. THRU is monophonic and runs to the end of file ----------------------
reset();
3 => s.playbackMode;        // THRU
s.noteOn(1, 127);
s.noteOn(2, 127);
1::samp => now;
check(s.activeVoices() == 1, "THRU chokes the previous voice");
(dur + 0.1)::second => now;
check(s.activeVoices() == 0, "THRU voice ends at end of file");

// --- 9. decay / fadeOut are accepted, clamped, and read back ----------------
reset();
0.3 => s.decay;  check(Math.fabs(s.decay() - 0.3) < 1e-9, "decay round-trips");
0.02 => s.fadeOut; check(Math.fabs(s.fadeOut() - 0.02) < 1e-9, "fadeOut round-trips");
-1.0 => s.decay; check(s.decay() == 0.0, "negative decay clamps to 0 (off)");
100.0 => s.fadeOut; check(s.fadeOut() <= 10.0, "fadeOut is clamped");
s.noteOn(1, 127);
100::ms => now;
check(s.activeVoices() == 1, "decay/fadeOut do not change when a slice voice ends (still active)");
200::ms => now;
check(s.activeVoices() == 0, "...and it still stops at the slice boundary");

// --- 10. degenerate loop points never crash or hang -------------------------
reset();
1 => s.playbackMode; 0.5 => s.loopStart; 0.5 => s.loopEnd; 2 => s.interp;
s.noteOn(60, 127);
200::ms => now;
0.6 => s.loopStart; 0.4 => s.loopEnd; 1 => s.reverse;
s.noteOn(61, 127);
200::ms => now;
check(true, "equal / crossed loop points render without crashing");
s.allNotesOff();

// --- 11. note positions past the end of file: voice ends at the end marker ---
reset();
s.notePos(3, dur + 5.0);
s.notePos(2, 0.5);           // slice 2 now [0.5, next = dur+5 s) -> clamped to end marker
s.noteOn(2, 127);
(dur - 0.5 + 0.1)::second => now;
check(s.activeVoices() == 0, "a note position beyond EOF does not leave a voice running past the file");
s.notePos(3, 0.0);

<<< "" >>>;
if (failures == 0) <<< "ALL PASS (test-semantics.ck)" >>>;
else <<< "FAILURES:", failures, "(test-semantics.ck)" >>>;
