DbMdaJX10 inst => dac;

<<<"Parameters", "">>>;
inst.printParams();

<<<"Presets", "">>>;
inst.printPresets();

for(int j;j<inst.getNumPresets();j++)
{
    <<<"Preset", j>>>;
    inst.selectPreset(j);
    for(int i;i<20;i++)
    {
        Math.random2(40, 60) => int mnote;
        inst.noteOn(mnote, 1.);
        .1::second => now;
        inst.noteOff(mnote, 1.);
    }
}


