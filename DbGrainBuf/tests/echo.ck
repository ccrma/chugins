<<<"Hello from DbGrain">>>;

// DbGrainBuf db => NRev rev => dac;
DbGrainBuf db => dac;
1 => db.debug;
"../../PitchTrack/data/obama.wav" => db.read;

<<<"repeating grains (2.5sec trigger, 2.5sec grain, so repeating)">>>;
.5 => db.gain; // lots of overlap
10 => db.grainPeriod; // seconds per grain (long)
// "blackman" => db.grainwindow;
"plancktaper" => db.grainwindow;
.35 => db.phasorStart;
.35 + 2.5 / db.filelen() => db.phasorStop; // fraction of the file == 2.5sec
1/2.5 => db.triggerFreq; // one trigger every 2.5 sec
15::second => now;

