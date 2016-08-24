// name: fau.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
Faust string => Faust distortion => Faust cryBaby => dac;
SinOsc LFO => blackhole;

1 => LFO.freq;

// evaluate faust code
string.compile("string.dsp");

cryBaby.eval(` 
process = crybaby_demo ;
`);

distortion.eval("
process = cubicnl_demo;
");

//cryBaby.dump();

// time loop
fun void notes(){
while( true )
{
    // set
    string.v("gate",0);
    10::ms => now;
    string.v("gate",1);
    string.v("freq", Math.random2f(80,800) );
    // advance time
    100::ms => now;
}
}

fun void lfoWah(){
    while(true){
        cryBaby.v("/CRYBABY/Wah_parameter",(LFO.last()*0.5+0.5));
        1::ms => now;
    }
}

spork ~ notes();
spork ~ lfoWah();

while(true){
    10::ms => now;
}    
