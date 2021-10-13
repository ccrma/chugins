DbVST3 x => dac;
.1 => float timescale;
x.loadPlugin("C:/Program Files/Common Files/VST3/examples/mda-vst3.vst3");

// EPiano
x.selectModule(12);  
playNotes(5);

// JX10
x.selectModule(14);  
playNotes(31);

// Piano
x.selectModule(20);  
playNotes(4);

// DX10 ---
x.selectModule(10);  
playNotes(31);

fun void playNotes(float npresets)
{
    for(int i;i<npresets;i++)
    {
        x.setParameter(0, i/npresets);
        for(int j;j<5;j++)
        {
            x.noteOn(40+j, .5);
            timescale * .5::second => now;
            x.noteOff(40+j, .5);
            timescale * 1::second => now;
            x.setParameter(1, .25);
        }
    }
}
