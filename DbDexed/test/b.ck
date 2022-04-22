DbDexed inst => dac;

inst.printPresets();

.1::second => dur noteDur;
[60, 62, 64, 65, 67, 69, 71] @=> int notes[];

for(int l;l<inst.getNumPresets();l++)
{
    <<<"Preset", l>>>;
    inst.selectPreset(l);
    for(int i;i<notes.size();i++)
    {
        notes[i] => int n;
        inst.noteOn(n, 1);
        noteDur => now;
        inst.noteOff(n, 1);
        noteDur => now;
    }
}

2::second => now; // ring


