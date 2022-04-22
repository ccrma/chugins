DbDexed inst => dac;

.25::second => dur noteDur;

for(int l;l<5;l++)
{
    inst.noteOn(60, 1);
    inst.noteOn(63, 1);
    inst.noteOn(66, 1);

    noteDur => now;

    inst.noteOff(60, 1);
    inst.noteOff(63, 1);
    inst.noteOff(66, 1);
    noteDur => now;
}

2::second => now; // ring
