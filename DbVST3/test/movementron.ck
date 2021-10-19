DbVST3 x => dac;
.1 => float timescale;

x.loadPlugin("Movementron.vst3");
<<<"Movementron loaded,", x.getNumParameters(), "parameters.">>>;
x.printModules();

<<<"100 randomly generated paramsets.">>>;
for(int i;i<100;i++)
{
    for(3 =>int j;j<x.getNumParameters();j++)
    {
        x.setParameter(j, Math.random2f(0.2, .8));
    }
    <<<i>>>;
    playNotes();
}

fun void playNotes()
{
    for(int j;j<5;j++)
    {
        x.noteOn(60+j, .5);
        timescale * .5::second => now;
        x.noteOff(60+j, .5);
        timescale * 1::second => now;
    }
}
