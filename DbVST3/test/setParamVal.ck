
//SawOsc s => DbVST3 x => dac;
SinOsc s => DbVST3 x => dac;
Math.random2f(250., 300.) => s.freq;
.1 => s.gain;
x.loadPlugin("ValhallaSupermassive.vst3");
<<<"module name", x.getModuleName()>>>; 
doAnim("Mode", 12, 1::second);

fun void doAnim(string name, int nsteps, dur stepdur)
{
    <<< "animating parameter", name >>>;
    for(int i;i<nsteps;i++)
    {
        (i $ float) / nsteps => float val;
        <<<i, val>>>;
        x.setParameter(name, val); 
        stepdur => now;
    }
}
