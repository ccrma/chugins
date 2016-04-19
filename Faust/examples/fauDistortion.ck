// name: fau.ck
// desc: demo of Faust chugin in action!

// instantiate and connect faust => ck
Faust fck => Faust distortion => Faust reverb => dac;
// evaluate faust code
fck.eval("import(\"music.lib\");
import(\"filter.lib\");
frequency = hslider(\"freq\",440,51,2000,0.01);
gain = hslider(\"gain\",1,0,1,0.01);
feedback = hslider(\"feedback\",1,0.9,1,0.01);
trig = button(\"gate\");
beta = hslider(\"pick_position\", 0.13, 0.02, 0.5, 0.01);
myString(freq,feedback) = +~(fdelay4(1024,delLength) <: (_+_')/2 : *(feedback))
with{
    delLength = SR/freq - 1;
};
noiseburst(g,P) = noise : *(g : trigger(P))*gain : pickposfilter
with {
    diffgtz(x) = (x-x') > 0;
    decay(n,x) = x - (x>0)/n;
    release(n) = + ~ decay(n);
    trigger(n) = diffgtz : release(n) : > (0.0);
    ppdel = beta*P; // pick position delay
    pickposfilter = ffcombfilter(4096,ppdel,-1); // defined in filter.lib
};
process = noiseburst(trig,(SR/frequency)) : lowpass(2,frequency) : myString(frequency,feedback);" );

// _<: and :> turn zita_rev_fdn_demo into a mono object
distortion.eval("import(\"effect.lib\"); 
drive = hslider(\"drive\",0, 0, 1, 0.01);
offset = hslider(\"offset\",0, 0, 1, 0.01);
process = cubicnl(drive,offset);
");
distortion.v("/0x00/drive",0.6);

reverb.eval("import(\"effect.lib\"); process = _ <: zita_rev_fdn_demo :> _ ;");

// time loop
while( true )
{
    // set
    fck.v("/0x00/gate",0);
    10::ms => now;
    fck.v("/0x00/gate",1);
    fck.v("/0x00/freq", Math.random2f(80,800) );
    // advance time
    500::ms => now;
}
