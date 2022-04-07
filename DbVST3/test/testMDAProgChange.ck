DbVST3 x => dac;
"mda-vst3.vst3" => string pluginName;
x.loadPlugin(pluginName);
while(!x.ready())
    1::ms => now;

x.printModules();
// x.setVerbosity(1);
x => dac;

.25::second => dur noteDur;
for(int j;j<2;j++)
{
    if(j == 0)
        x.selectModule(10); // DX10
    else
        x.selectModule(14); // JX10

    while(!x.ready())
        1::ms => now;
    <<<"Module ",  j==0 ? "DX10" : "JX10" >>>;

    x.setParameter("Factory Presets", 0.15);
    for(int i;i<10;i++)
    {
        x.noteOn(60, 1.);
        noteDur => now;
        x.noteOff(60, 0.);
    }

    <<<pluginName, "request preset">>>;
    x.setParameter("Factory Presets", 0.5);
    for(int i;i<10;i++)
    {
        x.noteOn(60, 1.);
        noteDur => now;
        x.noteOff(60, 0.);
    }
}

<<<pluginName, "done (in .ck)">>>;
