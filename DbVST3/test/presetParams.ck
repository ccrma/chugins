DbVST3 x => dac;
x.loadPlugin("C:/Program Files/Common Files/VST3/examples/mda-vst3.vst3");
x.selectModule(14);  // JX10
.2 => float timescale;
for(int i;i<51;i++)
{
    <<<"preset", i>>>;
    x.setParameter(0, i/51.0);
    for(int j;j<5;j++)
    {
        x.noteOn(40+j, .5);
        timescale * .5::second => now;
        x.noteOff(40+j, .5);
        timescale * 1::second => now;
        x.setParameter(1, .25);
    }
}
