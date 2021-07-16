<<<"Hello from DbGrain">>>;

// DbGrainBuf db => NRev rev => dac;
DbGrainBuf db => dac;
"../../PitchTrack/data/obama.wav" => db.read;

<<<"forward grainRate">>>;
1.1 => db.grainRate;
.5 => db.phasorStart;
5::second => now;

<<<"reversed grainRate">>>;
-1.1 => db.grainRate;
.5 => db.phasorStart;
5::second => now;

<<<"fwd grainRate, reversed phaseRate">>>;
0 => db.debug;
1.0 => db.grainRate;
.5 => db.phasorStart;
.8 => db.phasorStop;
-.5 => db.phasorRate;
5::second => now;

