DbVST3 x;

// "NA Black.vst3" => string pluginName;
//"examples/againsimple.vst3" => string pluginName;
"examples/mda-vst3.vst3" => string pluginName;
x.loadPlugin("C:/Program Files/Common Files/VST3/" + pluginName);
x.getNumModules() => int nmods;
if(nmods > 1)
{
    x.printModules();
    x.selectModule(24); // mda: 31 is flang, 24 is roundpanner
}

<<< pluginName, "parameters:">>>;
for(int i; i<x.getNumParameters();i++)
{
    <<<i, x.getParameterName(i)>>>;
}

/* filter test... seems like roundpanner isn't stereo? */
SqrOsc n => Pan2 p => x => dac;
.25 => n.gain;
2::second => now;
