DbVST3 inst => dac;
inst.loadPlugin("mda-vst3.vst3");
while(!inst.ready())
    1::ms => now;
inst.selectModule(14);
while(!inst.ready())
    1::ms => now;

inst.setParameter("Factory Presets", 20);

for(int i;i<20;i++)
{
    Math.random2(40, 60) => int mnote;
    inst.noteOn(mnote, 1.);
    .1::second => now;
    inst.noteOff(mnote, 1.);
}

