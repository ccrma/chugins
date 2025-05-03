@import "FluidSynth.chug"

// synchronize to period
0.75::second => dur T;
T - (now % T) => now;

// connect patch
FluidSynth f => NRev reverb => dac;
f.open(me.dir() + "HS_African_Percussion.sf2");
.75 => f.gain;
0.02 => reverb.mix;

// scale (in semitones)
[ 0, 2, 4, 7, 9 ] @=> int scale[];

f.progChange(0);     // piano on MIDI channel 1
f.progChange(6, 1);  // harpsicord on MIDI channel 2
f.progChange(11, 2); // vibraphone on MIDI channel 3
f.setBank(128, 3);
f.progChange(0, 3);  // standard drumkit on MIDI channel 4

// infinite time loop
while( true )
{
    scale[Math.random2(0,4)] => int freq;
    // get the final freq
    45 + (Math.random2(0,3)*12 + freq) => int note;
    
    // choose a channel we've set
    Math.random2(0,3) => int chan;
    f.noteOn(note, 100, chan);
    
    // advance time
    .25::T => now;
}

