<<<"Hello from DbGrain">>>;

// DbGrainBuf db => NRev rev => dac;
DbGrainBuf db => dac;
"../../PitchTrack/data/obama.wav" => db.read;

<<<"GrainBuf with Defaults">>>;
20::second => now;

