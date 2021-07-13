<<<"Hello from DbGrain">>>;

// DbGrainBuf db => NRev rev => dac;
DbGrainBuf db => dac;
"../../PitchTrack/data/obama.wav" => db.read;

<<<"reversed grainRate">>>;
-1.1 => db.grainRate;
.5 => db.phasorStart;
5::second => now;

<<<"reversed phaseRate, fwd grainRate">>>;
1.0 => db.grainRate;
.4 => db.phasorStart;
-.5 => db.phasorRate;
5::second => now;

