
SawOsc s => DbVST3 x => dac;
Math.random2f(250., 300.) => s.freq;
.1 => s.gain;
x.loadPlugin("C:/Program Files/Common Files/VST3/examples/mda-vst3.vst3");

x.selectModule(31);  // Thru-zero flanger
doAnim();

x.selectModule(23);  // RingMod
doAnim();

x.selectModule(24);  // Round Panner
doAnim();

fun void doAnim()
{
    <<<x.getModuleName()>>>; 
    x.getNumParameters() => int nump;
    for(int i; i<nump;i++)
    {
        <<<"  ", i, x.getParameterName(i)>>>;
    }
    for(1=>int p;p<nump;p++)
    {
        <<< "animating parameter", p >>>;
        for(int i;i<200;i++)
        {
            // for now we'll operate with normaized values... 
            // 10 + i * 10 => float rate;
            (i%10) / 9. => float rate;
            x.setParameter(p, rate); // parameter 1 is Rate fro flanger
            .025::second => now;
        }
    }
}
