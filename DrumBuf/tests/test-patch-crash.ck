//-----------------------------------------------------------------------------
// Minimal test to isolate Patch crash
//-----------------------------------------------------------------------------

DrumBuf snare => dac;

if (snare.read(me.dir() + "/snare.wav") == 0) {
    <<< "FAIL: could not load snare.wav" >>>;
    me.exit();
}

<<< "Testing Patch with continuous modulation..." >>>;

// Setup
0.02 => snare.grainSize;
1 => snare.envelope;
0.001 => snare.attack;
0.08 => snare.release;

// Create LFO modulating transpose
SinOsc pitchLFO => Patch pitchMod(snare, "transpose") => blackhole;
2.0 => pitchLFO.freq;
5.0 => pitchLFO.gain;

<<< "Starting rapid notes with LFO..." >>>;

// Just 8 notes instead of 32
for (0 => int i; i < 8; i++) {
    snare.noteOn(60, 110);
    62.5::ms => now;
    snare.noteOff();
}

<<< "Notes complete, disconnecting Patch..." >>>;
pitchMod.disconnect();

<<< "Patch disconnected, waiting..." >>>;
1::second => now;

<<< "Test complete!" >>>;
