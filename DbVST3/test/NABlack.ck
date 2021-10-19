DbVST3 y;
DbVST3 x => y => dac;
float timescale;

x.loadPlugin("mda-vst3.vst3");
x.selectModule(12); // electric piano (which has 5-ish presets)

y.loadPlugin("NA Black.vst3");
<<<"NA Black loaded,", x.getNumParameters(), "parameters.">>>;
y.printModules();

<<<"Default settings">>>;
.5 => timescale;
playNotes();
<<<"Power off">>>;
y.setParameter("Power", 0);
playNotes();
<<<"Power on">>>;
y.setParameter("Power", 1);
playNotes();
<<<"Bypass 1">>>;
y.setParameter("Bypass", 1);
playNotes();
<<<"Bypass 0">>>;
y.setParameter("Bypass", 0);
playNotes();

<<<"100 randomly generated paramsets.">>>;
.08 => timescale;
for(int i;i<100;i++)
{
    // last two params are boolean Power and Bypass
    for(int j;j<y.getNumParameters()-2;j++)
    {
        y.setParameter(j, Math.random2f(0., 1.0));
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
