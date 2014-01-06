
// synchronize to period
0.75::second => dur T;
T - (now % T) => now;

// connect patch
FluidSynth f => NRev reverb => dac;
f.open("/usr/share/sounds/sf2/FluidR3_GM.sf2");
.25 => f.gain;
0.02 => reverb.mix;

// scale (in semitones)
[ 0, 2, 4, 7, 9 ] @=> int scale[];

// infinite time loop
while( true )
{
    scale[Math.random2(0,4)] => int freq;
    // get the final freq
    45 + (Math.random2(0,3)*12 + freq) => int note;
    
    f.noteOn(note, 100);
    
    // advance time
    .25::T => now;
}

