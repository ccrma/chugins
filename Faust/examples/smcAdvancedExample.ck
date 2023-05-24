// name: smcAvancedExample.ck
// desc: advanced FaucK example! 

// a string connected to a "cry baby" effect
Faust string => Faust cryBaby => dac;

// LFO to modulate cryBaby
SinOsc LFO => blackhole;
6 => LFO.freq;

// string is implemented in an external Faust file
string.compile(me.dir() +"string.dsp");

// crybaby_demo is part of the effect.lib Faust library
cryBaby.eval(` 
  process = dm.crybaby_demo ;
`);

// generates random notes
fun void notes(){
    while( true ){
        // new note
        string.v("gate",0);
        10::ms => now;
        string.v("gate",1);
        // with random frequency
        string.v("freq", Math.random2f(80,800) );
        100::ms => now;
    }
}

// modulates the cry baby with the LFO 
fun void lfoWah(){
    while(true){
        cryBaby.v("/CRYBABY/Wah_parameter",(LFO.last()*0.5+0.5));
        1::samp => now;
    }
}

spork ~ notes();
spork ~ lfoWah();

while(true){
    10::ms => now;
}    
