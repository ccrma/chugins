DbVST3 x;

// "NA Black.vst3" => string pluginName;
//"examples/againsimple.vst3" => string pluginName;
"examples/mda-vst3.vst3" => string pluginName;
x.loadPlugin("C:/Program Files/Common Files/VST3/" + pluginName);
<<< pluginName, "loaded">>>;
x.getNumModules() => int nmods;
if(nmods > 1)
{
    x.printModules();
    selectModule(31); // flanger
}

SqrOsc n => x => dac; /* don't need pan 2 atm monoin is up-channed */
.1 => n.gain;
5::second => now;

if(nmods > 1)
{
    selectModule(24);
    5::second => now;
}

fun void selectModule(int i)
{
    if(i != -1)
        x.selectModule(i); 
    <<< "Set module:", x.getModuleName(), "----" >>>;
    for(int i; i<x.getNumParameters();i++)
    {
        <<<"  ", i, x.getParameterName(i)>>>;
    }
}
