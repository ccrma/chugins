DbVST3 x;
"AudioPluginDemo.vst3" => string pluginName;
x.loadPlugin(pluginName);
while(!x.ready())
    1::ms => now;
<<< pluginName, "loaded, ", x.getNumModules(), "modules.">>>;
x.printModules();

x => dac;

x.noteOn(60, 1.);
2::second => now;

x.noteOff(60, 0.);
2::second => now;


