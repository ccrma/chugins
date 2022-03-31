DbVST3 @x;

new DbVST3 @=> x;
"LABS (64 Bit).vst3" => string pluginName;
x.loadPlugin(pluginName);
while(!x.ready())
    1::ms => now;
// <<< "testLABS.ck", pluginName, "loaded, ", x.getNumModules(), "modules.">>>;
// x.printModules();

<<< "make a sound", "please">>>;
x => dac;
x.noteOn(60, 1.);
2::second => now;
x.noteOff(60, 0.);
2::second => now;

<<<pluginName, "done (in .ck)">>>;

