DbDexed inst => dac;

inst.noteOn(40, 1);
inst.noteOn(43, 1);
inst.noteOn(46, 1);

1::second => now;

inst.noteOff(40, 1);
inst.noteOff(43, 1);
inst.noteOff(46, 1);
1::second => now;


