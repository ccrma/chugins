<<<"Hello from DbGrain">>>;

// DbGrainBuf db => NRev rev => dac;
DbGrainBuf db => dac;
1 => db.debug;
"../../PitchTrack/data/obama.wav" => db.read;

<<<"tiny grain as tone">>>;
.09 => db.grainPeriod; // seconds per grain (long)
.35 => db.phasorStart;
.35 => db.phasorStop;
10 => db.triggerFreq; // 10 triggers per second

15::second => now;

