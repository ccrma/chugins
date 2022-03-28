DbVST3 x;
"mda-vst3.vst3" => string pluginName;
x.loadPlugin(pluginName);
while(!x.ready())
    1::ms => now;
<<< pluginName, "loaded">>>;

x.getNumModules() => int nmods;
if(nmods > 1)
{
    x.printModules();
    selectModule(31); // flanger
}

SqrOsc n => x => dac; /* don't need pan 2 atm monoin is up-channed */
.1 => n.gain;
3::second => now;
<<<"Rate", .8>>>;
x.setParameter("Rate", .8); // normalized values
2::second => now;
<<<"DepthMod", 1>>>;
x.setParameter("DepthMod", 1); 
2::second => now;

if(nmods > 1)
{
    selectModule(24);
    5::second => now;
}

fun void selectModule(int i)
{
    if(i != -1)
    {
        x.selectModule(i); 
        while(!x.ready())
            1::ms => now;

        <<< "Set module:", x.getModuleName(), "----" >>>;
    }
    for(int i; i<x.getNumParameters();i++)
    {
        <<<"  ", i, x.getParameterName(i)>>>;
    }
}
