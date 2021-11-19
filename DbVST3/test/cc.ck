int done;
fun void doit()
{
    "mda-vst3.vst3" => string instname;
    14 => int mod; // 10:DX, 14:JX

    <<< "Instrument", instname, "module", mod >>>;

    DbVST3 inst;
    inst.setVerbosity(2);
    inst.loadPlugin(instname);
    if(mod != -1)
        inst.selectModule(mod);

    // inst => filt => dac;
    inst => dac;

    for(int i;i<20;i++)
    {
        Math.random2(40, 60) => int note;
        inst.noteOn(note, .8);
        for(float j; j<2; .1 +=> j)
        {
            Math.fmod(j, 1) => float val;
            inst.setParameter("PitchBend", val);
            // inst.setParameter("Mod Wheel", val);
            // inst.setParameter("Aftertouch", val);
            .05::second => now;
        }

        .5::second => now;
        inst.noteOff(note, .8);
    }
    1 => done;
}

spork ~ doit();

while(!done)
    1::second => now;

<<< "Done" >>>;
