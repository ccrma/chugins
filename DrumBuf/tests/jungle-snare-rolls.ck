//-----------------------------------------------------------------------------
// Jungle Snare Rolls with Pitch-Up Effect
// Using DrumBuf with sporked pitch automation for classic jungle/dnb rolls
//-----------------------------------------------------------------------------

DrumBuf snare => dac;

if (snare.read(me.dir() + "/snare.wav") == 0) {
    <<< "FAIL: could not load snare.wav" >>>;
    me.exit();
}

// Global variables for LFO patterns
SinOsc pitchLFO => blackhole;
SinOsc reeseLFO => blackhole;

<<< "=== JUNGLE SNARE ROLLS ===" >>>;
<<< "" >>>;

// Setup for tight, punchy jungle snares
0.02 => snare.grainSize;  // Small grains for crisp sound
-3.0 => snare.gain;        // A bit quieter for the rolls

// Enable envelope for tight snare hits
1 => snare.envelope;
0.001 => snare.attack;     // Instant attack
0.08 => snare.release;     // Short, punchy release

// Helper function for LFO pitch modulation (Pattern 7)
fun void updatePitchFromLFO() {
    while (true) {
        pitchLFO.last() * 5.0 => snare.transpose;  // +/- 5 semitones
        1::samp => now;
    }
}

// Helper function for Reese bass wobble (Pattern 10)
fun void updateReesePitch() {
    while (true) {
        reeseLFO.last() * 3.0 => snare.transpose;  // +/- 3 semitones
        1::samp => now;
    }
}

// Helper function for discrete step rolls
fun void snareRoll(float startPitch, float endPitch, dur rollDur, int numHits) {
    rollDur / numHits => dur hitInterval;
    (endPitch - startPitch) / numHits => float pitchStep;

    for (0 => int i; i < numHits; i++) {
        startPitch + (pitchStep * i) => snare.transpose;

        // Velocity increases slightly through the roll for dynamics
        100 + (27 * i / numHits) $ int => int vel;

        snare.noteOn(60, vel);
        hitInterval => now;
        snare.noteOff();
    }
}

// Helper function for smooth pitch automation
fun void snareRollWithGlide(float startPitch, float endPitch, dur rollDur, int numHits) {
    rollDur / numHits => dur hitInterval;
    now => time startTime;

    for (0 => int i; i < numHits; i++) {
        // Smoothly interpolate pitch based on elapsed time
        (now - startTime) / rollDur => float progress;
        if (progress > 1.0) 1.0 => progress;
        startPitch + ((endPitch - startPitch) * progress) => snare.transpose;

        100 + (27 * i / numHits) $ int => int vel;
        snare.noteOn(60, vel);
        hitInterval => now;
        snare.noteOff();
    }
}

<<< "Pattern 1: Classic 16th note roll with 5 semitone pitch-up" >>>;
snareRoll(0.0, 5.0, 1::second, 16);
1::second => now;

<<< "" >>>;
<<< "Pattern 2: Fast 32nd note roll with big pitch sweep (0 to 12 semitones)" >>>;
snareRoll(0.0, 12.0, 1::second, 32);
1::second => now;

<<< "" >>>;
<<< "Pattern 3: Triplet roll with subtle pitch-up" >>>;
snareRoll(0.0, 3.0, 0.5::second, 12);
1.5::second => now;

<<< "" >>>;
<<< "Pattern 4: Double-time rolls (the classic jungle stutter)" >>>;
// First roll
snareRoll(0.0, 7.0, 0.5::second, 16);
0.5::second => now;
// Second roll, higher starting pitch
snareRoll(3.0, 10.0, 0.5::second, 16);
1::second => now;

<<< "" >>>;
<<< "Pattern 5: Smooth pitch automation with sporked glide" >>>;
snareRollWithGlide(0.0, 8.0, 1::second, 16);
1::second => now;

<<< "" >>>;
<<< "Pattern 6: The full jungle breakbeat drop simulation!" >>>;
// Build up tension with increasingly faster rolls
<<< "  Building tension..." >>>;
snareRoll(-2.0, 3.0, 1::second, 8);
0.5::second => now;
snareRoll(-2.0, 5.0, 0.75::second, 12);
0.25::second => now;
snareRoll(0.0, 7.0, 0.5::second, 16);
0.25::second => now;

<<< "  DROP!" >>>;
snareRoll(0.0, 12.0, 0.5::second, 32);

<<< "" >>>;
1::second => now;

<<< "" >>>;
<<< "Pattern 7: Pitch LFO for wobbly jungle rolls" >>>;

// Configure the LFO to modulate pitch continuously
2.0 => pitchLFO.freq;  // 2 Hz wobble

// Spork the LFO shred
spork ~ updatePitchFromLFO() @=> Shred pitchShred;

// Play a continuous roll while the LFO wobbles the pitch
for (0 => int i; i < 32; i++) {
    snare.noteOn(60, 110);
    62.5::ms => now;  // 16th notes at 120 BPM
    snare.noteOff();
}

// Stop the LFO shred
Machine.remove(pitchShred.id());
0.0 => snare.transpose;  // Reset pitch

<<< "" >>>;
1::second => now;

<<< "" >>>;
<<< "Pattern 8: Exponential pitch ramp for aggressive drops" >>>;

// Exponential pitch-up curve
[0.0, 0.5, 1.5, 3.0, 5.0, 7.5, 10.5, 15.0] @=> float pitches[];

for (0 => int i; i < pitches.size(); i++) {
    pitches[i] => snare.transpose;

    // Play 4 hits at each pitch level
    for (0 => int j; j < 4; j++) {
        snare.noteOn(60, 120);
        40::ms => now;
        snare.noteOff();
    }
}

0.0 => snare.transpose;  // Reset pitch

<<< "" >>>;
1::second => now;

<<< "" >>>;
<<< "Pattern 9: Ultra-smooth glide with continuous triggering" >>>;

// Create a very smooth pitch glide
-5.0 => float glideStart;
15.0 => float glideEnd;
2::second => dur glideDur;
now => time glideStartTime;

// Rapidly trigger snares while pitch smoothly glides up
for (0 => int i; i < 64; i++) {
    // Calculate smooth pitch based on elapsed time
    (now - glideStartTime) / glideDur => float progress;
    if (progress > 1.0) 1.0 => progress;
    glideStart + ((glideEnd - glideStart) * progress) => snare.transpose;

    snare.noteOn(60, 105 + (i % 20));
    31.25::ms => now;  // 32nd notes
    snare.noteOff();
}

0.0 => snare.transpose;  // Reset pitch

<<< "" >>>;
1::second => now;

<<< "" >>>;
<<< "Pattern 10: The 'Reese Bass' effect - slow wobble with fast rolls" >>>;

// Configure very slow pitch wobble
0.5 => reeseLFO.freq;  // Slow wobble

// Spork the Reese wobble shred
spork ~ updateReesePitch() @=> Shred reeseShred;

// Fast rolls while pitch wobbles slowly
for (0 => int i; i < 4; i++) {
    // 16 note burst
    for (0 => int j; j < 16; j++) {
        snare.noteOn(60, 100 + (j * 2));
        50::ms => now;
        snare.noteOff();
    }
    200::ms => now;  // Gap between bursts
}

Machine.remove(reeseShred.id());
0.0 => snare.transpose;  // Reset pitch

<<< "" >>>;
1::second => now;

<<< "" >>>;
<<< "=== JUNGLE SNARE ROLL DEMO COMPLETE ===" >>>;
<<< "" >>>;
<<< "Techniques demonstrated:" >>>;
<<< "  1. Basic discrete pitch-up rolls (jungle staple)" >>>;
<<< "  2. Fast 32nd note rolls for drops" >>>;
<<< "  3. Double-time stuttering patterns" >>>;
<<< "  4. Smooth pitch automation with sporked shreds" >>>;
<<< "  5. LFO-driven wobbly pitch effects" >>>;
<<< "  6. Exponential pitch curves for aggressive drops" >>>;
<<< "  7. Sample-rate smooth continuous pitch glides" >>>;
<<< "  8. Slow wobble with fast rolls ('Reese' effect)" >>>;
<<< "" >>>;
<<< "Try tweaking:" >>>;
<<< "  - snare.grainSize for different textures (smaller = grittier)" >>>;
<<< "  - Attack/release times for tighter or looser hits" >>>;
<<< "  - Roll duration and note counts" >>>;
<<< "  - Pitch ranges and curves" >>>;
<<< "  - LFO frequencies and depths" >>>;
<<< "  - Glide update rates (1::ms vs 1::samp)" >>>;
