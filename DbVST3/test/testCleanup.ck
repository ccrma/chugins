DbVST3 x;

"AudioPluginDemo.vst3" => string pluginName;
x.loadPlugin(pluginName);
while(!x.ready())
    1::ms => now;
<<< "testCleanup.ck", pluginName, "loaded, ", x.getNumModules(), "modules.">>>;
x.printModules();

//DbVST3 x2;
//x2.loadPlugin(pluginName);
//while(!x2.ready())
    //1::ms => now;

x => dac;

x.noteOn(60, 1.);
2::second => now;

1::ms => now;

x.noteOff(60, 0.);
2::second => now;

<<<pluginName, "done (in .ck)">>>;
