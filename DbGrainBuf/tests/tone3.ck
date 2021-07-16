<<<"Hello from DbGrain">>>;

// DbGrainBuf db => NRev rev => dac;
DbGrainBuf db => dac;

0 => db.debug;
"../../PitchTrack/data/obama.wav" => db.read;

<<<"word scale">>>;
"plancktaper95" => db.grainWindow;
49.6 => float start;
.35::second => db.grainPeriod; // seconds per grain 
start => db.phasorStartSec;
1 => db.grainRate;
start + (db.grainPeriod() / 1::second) => db.phasorStopSec;
1::second / (.5*db.grainPeriod()) => db.triggerFreq;  // two triggers per grain
5::second => now;

<<<"word scale, triggerRange:.75 (randomized trigging)">>>;
.75 => db.triggerRange; 
5::second => now;
