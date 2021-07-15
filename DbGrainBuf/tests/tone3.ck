<<<"Hello from DbGrain">>>;

// DbGrainBuf db => NRev rev => dac;
DbGrainBuf db => dac;

0 => db.debug;
"../../PitchTrack/data/obama.wav" => db.read;

<<<"word scale">>>;
"plancktaper95" => db.grainWindow;
49.6 => float start;
.35 => db.grainPeriod; // seconds per grain 
start => db.phasorStartSec;
1 => db.grainRate;
start + db.grainPeriod() => db.phasorStopSec;
1 / (.5*db.grainPeriod()) => db.triggerFreq; 
15::second => now;

