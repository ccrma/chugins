DbVST3 x => dac;
"mda-vst3.vst3" => string pluginName;
x.loadPlugin(pluginName);
while(!x.ready())
    1::ms => now;

// x.printModules();
x.selectModule(10);

x => dac;
for(int i;i<10;i++)
{
    x.noteOn(60, 1.);
    .1::second => now;
    x.noteOff(60, 0.);
}

<<<pluginName, "request preset">>>;
x.setParameter("Factory Presets", 0.25);
for(int i;i<10;i++)
{
    x.noteOn(60, 1.);
    .1::second => now;
    x.noteOff(60, 0.);
}

<<<pluginName, "done (in .ck)">>>;
