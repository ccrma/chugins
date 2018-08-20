/*
This file documents and shows examples of how I have implemented FluidSynth's tuning functions in this chugin.

FluidSynth's implementation allows you to set the tuning for a tuning bank and progam (similar in concept to 
the banks and programs for instruments, but independent), and then you must assign the tuning bank/program to
a MIDI channel in order to use it.

I wanted to just be able to set the tuning for a MIDI channel directly; without having to worry about where FluidSynth
stored it. Or (more likely) set _one_ tuning for _all_ MIDI channels. So that is the way I made these hooks work.

I used ChucK's polymorphism to implement all tuning functions to work in this way:
- If a MIDI channel is specified, the tuning is affected for that channel only. (The tuning bank is 0 and the tuning
program is equal to the zero-based channel number, but you usually shouldn't need to know this.)
- If no MIDI channel is provided, the tuning is set for _all_ MIDI channels. (The tuning bank/program is 0/0 and all
channels are set to use this, but again, you usually shouldn't need to care.)

Forrest Cahoon (forrest.cahoon@gmail.com)
*/

// set up a FluidSynth object
FluidSynth f => dac;
f.open("/usr/share/sounds/sf2/FluidR3_GM.sf2");
.75 => f.gain;

// A simple function for us to test with
fun void playChords(int chan)
{
    f.noteOn(60, 100, chan);
    0.5::second => now;
    f.noteOn(64, 100, chan);
    0.5::second => now;
    f.noteOn(67, 100, chan);
    1.5::second => now;

    f.noteOn(60, 100, chan);
    0.5::second => now;
    f.noteOn(63, 100, chan);
    0.5::second => now;
    f.noteOn(67, 100, chan);
    0.5::second => now;
}

/*
setOctaveTuning() is for setting new tunings which have 12 notes per octave.
The tuning is passed in as an array of 12 floats containing the cents deviation
from standard tuning for each note, starting with C.

The following example shows how to set a tuning from a set of ratios.
*/

// The ratios for our tuning
[     1., 16./15.,  9./8.,   6./5., 
   5./4.,   4./3., 45./32.,  3./2.,
   8./5.,   5./3.,  9./5.,   15./8. ] @=> float justIntonationRatios[];

float justIntonation[12]; // This will hold our cents deviation

// For each note, convert the ratio to cents and subtract the standard tuning cents 
for (0 => int i; i < justIntonation.cap(); i++) {
    1200. * Math.log2(justIntonationRatios[i]) - (100. * i) => justIntonation[i];
}

// Raise each note in our scale by a quarter tone (50 cents)
// just to make the tuning difference obvious
float justIntonationHigh[12];
for (0 => int i; i < justIntonation.cap(); i++) {
    justIntonation[i] + 50.0 => justIntonationHigh[i];
}

// Omitting a channel number here will set the tuning
// for all MIDI channels
f.setOctaveTuning(justIntonationHigh);

f.progChange(6, 1); // harpsicord on MIDI channel 2

// Play the same JI scale on different instruments
spork ~ playChords(0);
0.25::second => now;
spork ~ playChords(1);
5::second => now;

// Omitting the channel number here resets all MIDI channels
// to the standard equal temperament
f.resetTuning();

// Play the same equal temperament scale on different instruments
spork ~ playChords(0);
0.25::second => now;
spork ~ playChords(1);
5::second => now;

// Specifying the channel number sets the tuning only on that channel
f.setOctaveTuning(justIntonationHigh, 0);

// Play just intonation on piano; equal temperament on vibraphone
spork ~ playChords(0);
0.25::second => now;
spork ~ playChords(1);
5::second => now;

/*
setTuning() is used to set the tuning for all 128 MIDI note numbers.

It takes an array of 128 floats, which are the pitches for each MIDI note number 
in cents above the standard tuning of MIDI note 0. Another way of saying that is
1000 * the MIDI note number of the next lowest note in standard MIDI tuning, 
plus the cents above that note.

Here we set a 19-tone equal temperament tuning, starting at the C which is MIDI
note number 24 in standard tuning.
*/
1200.0/19.0 => float interval; // Divide the octave (1200 cents) into 19 equal divisions
float nineteenTuning[128];
for (0 => int i; i < nineteenTuning.cap(); i++) {
    2400. + (interval * i) => nineteenTuning[i];
}

f.setTuning(nineteenTuning); // This sets the tuning for _all_ MIDI channels, because no channel is specified


// Play triads with all the different 3rds in 19tET

for (61 => int i; i <= 64; i++) {
    f.noteOn(57, 100, 0);
    0.25::second => now;
    f.noteOn(i, 100, 0);
    0.25::second => now;
    f.noteOn(68, 100, 0);
    0.75::second => now;
}
1::second => now;

// Now reset the tuning for all channels.
f.resetTuning();

/*
tuneNote() is used to set the tuning of individual notes, taking as arguments the MIDI note number,
the pitch in cents above the standard tuning of MIDI note 0 (same as setTuning), and optionally a
zero-based MIDI channel number. (As with all of these methods, specifying the channel number sets
the tuning for only that channel; omitting it sets the tuning for all channels.)

For an example we use the MIDI note 0 to store a natural seventh above middle C.
*/

// start with just intonation
f.setOctaveTuning(justIntonation);

// Get the pitch that's 7/4 above the standard tuning of MIDI note 60
// (middle C, which is also the same in our just intonation tuning).
6000. + (1200. * Math.log2(7./4.)) => float naturalSeventh;

// set MIDI note 0 (which is pretty much unusable otherwise) to our new pitch
f.tuneNote(0, naturalSeventh);

f.noteOn(60, 100, 0);
0.25::second => now;
f.noteOn(64, 100, 0);
0.25::second => now;
f.noteOn(67, 100, 0);
0.25::second => now;
f.noteOn(0, 100, 0); // use it in a chord
2::second => now;

/*
Finally, there is tuneNotes(), which sets pitches for multiple MIDI note numbers at once.
The arguments are the same as tuneNote(), except it takes an array of MIDI note numbers and
an array of pitches (as cents above the standard tuning of MIDI note 0) instead of an individual
MIDI note number and pitch. Like all of the tuning functions, it optionally takes a zero-based
MIDI channel number, which when specified sets the tuning for only that channel, but when omitted
sets the tuning for all channels.
*/

// use low MIDI note numbers for natural sevenths in several octaves
f.tuneNotes([0, 1, 2], [naturalSeventh - 1200., naturalSeventh, naturalSeventh + 1200.]);

for (0 => int i; i < 3; i++) {
    f.noteOn(48 + (12*i), 100, 0);
    0.25::second => now;
    f.noteOn(52 + (12*i), 100, 0);
    0.25::second => now;
    f.noteOn(55 + (12*i), 100, 0);
    0.25::second => now;
    f.noteOn(i, 100, 0);
    0.25::second => now;
}
2::second => now;