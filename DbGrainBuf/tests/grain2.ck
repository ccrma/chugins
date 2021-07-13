<<<"Hello from DbGrain">>>;

// DbGrainBuf db => NRev rev => dac;
DbGrainBuf db => dac;
"../../PitchTrack/data/obama.wav" => db.read;

<<<"GrainBuf with slower grainRate">>>;
.9 => db.grainRate;
.5 => db.grainPhaseStart;
5::second => now;

<<<"with faster grainRate">>>;
1.1 => db.grainRate;
.5 => db.grainPhaseStart;
5::second => now;

