/*
This is an example of using MIDI pitch bend.

The value of pitch bend passed by MIDI is in the range 0 to 16383 with 8192 being center.

The libfluidsynth hooks are:

setPitchBend(int value, int chan);   // Sets the pitch bend of zero-based MIDI channel chan
getPitchBend(int chan) => int value; // Retrieves the pitch bend value of zero-based MIDI channel chan
resetPitchBend(int chan);            // Equivalent to setPitchBend(chan, 8192), effectively "turns off"
                                     // pitch bend for zero-based MIDI channel chan

Each of these has a corresponding version in which the channel is not specified. These work on the default
channel 0, except for resetPitchBend() which "turns off" MIDI pitch bend for ALL MIDI channels:

setPitchBend(int value);
getPitchBend() => int value;
resetPitchBend();

Values less than the minimum value of 0 or more than the maximum value of 16383 are set equal to 0 or 16383
respectively. This ensures a small rounding error won't stop your program.
*/

// Here's a function which manipulates the pitch bend in a sinusoidal manner:
fun void pitchWobble(FluidSynth f, dur duration, int cycles, int chan) {
    (duration/1::samp) $ int => int sampleLength;
    int pitchBend;
    for (0 => int i; i < sampleLength; i++) {
        (8192 * (1.0 + Math.sin(2*cycles*i*Math.PI/sampleLength))) $ int => pitchBend;
        f.setPitchBend(pitchBend);
        1::samp => now;
    }
}

// Here's version which doesn't specify the MIDI channel: 
fun void pitchWobble(FluidSynth f, dur duration, int cycles) {
    pitchWobble(f, duration, cycles, 0);
}

// set up a FluidSynth object
FluidSynth f => dac;
f.open("/usr/share/sounds/sf2/FluidR3_GM.sf2");
.75 => f.gain;

// First let's just use the default MIDI channel:

f.progChange(73); // Flute


f.noteOn(60,  60);
f.noteOn(64,  80);
f.noteOn(67, 100);
pitchWobble(f, 2::second, 4);


f.noteOff(60);
f.noteOff(64);
f.noteOff(67);

// What is the pitch bend value after we're done?
// On my system, it's not quite the default 8192 (although it's close)
<<< "Pitch bend value is now ", f.getPitchBend() >>>;

// It's a good idea to reset the pitch bend, for just such calculation discrepencies.
f.resetPitchBend(); 

// Here's a multi-channel version:
f.progChange(73, 1);
f.progChange(73, 2);

f.noteOn(60,  60, 0);
f.noteOn(64,  80, 1);
f.noteOn(67, 100, 2);

spork ~ pitchWobble(f, 4::second, 5, 0);
spork ~ pitchWobble(f, 4::second, 6, 1);
spork ~ pitchWobble(f, 4::second, 7, 2);

4::second => now;

f.noteOff(60, 0);
f.noteOff(64, 1);
f.noteOff(67, 2);

// This will reset the pitch bend for ALL MIDI channels:
f.resetPitchBend();
