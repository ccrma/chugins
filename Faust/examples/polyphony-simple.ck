// name: polyphony-simple.ck
// desc: Polyphonic sawtooth synthesizer.
Faust faust => dac;

10 => faust.numVoices;

faust.eval(`
    import("stdfaust.lib");
    freq = hslider("freq",200,50,1000,0.01);
    gain = hslider("gain",0.5,0,1,0.01) * .1;
    gate = button("gate");
    process = os.sawtooth(freq)*gain*gate;
    effect = _, _; // remember to declare effect when using polyphony
`);

faust.dump();

fun void playNote(int pitch, int vel, dur delay, dur duration) {
    delay => now;
    faust.noteOn(pitch, vel);
    duration => now;
    faust.noteOff(pitch, 0);
}

fun void playChord(int chord[], int vel, dur duration) {
    for (0 => int i; i < chord.size(); i++) {
        Math.random2f(0.01,0.4)::second => dur delay;
        spork~ playNote(chord[i], vel, delay, duration);
    }
}

//  Some chords.  We don't use all here
[41, 56, 60, 63, 67] @=> int Fmi9[];
[36, 55, 58, 62, 65] @=> int BfMajC[];
[34, 56, 60, 63, 67] @=> int AfMaj7Bf[];
[43, 53, 58, 62, 65] @=> int Gmi7[];
[62, 66, 69, 74] @=> int DMaj[];
[59, 62, 66, 68] @=> int Bmi6[];
[55, 59, 62, 66] @=> int GMaj7[];
[59, 62, 66, 71] @=> int Bmi[];
[62, 66, 69, 73] @=> int D7[];
[62, 66, 69, 74] @=> int DFs[];

1.0::second => dur t;

// infinite time loop
while( true )
{
    playChord(DMaj, 127, t);
    t * 1.5 => now;

    playChord(Gmi7, 80, t);
    t * 1.5 => now;
}