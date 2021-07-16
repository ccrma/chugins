<<<"Hello from DbGrain">>>;

// DbGrainBuf db => NRev rev => dac;
DbGrainBuf db => dac;
.5 => db.gain;

0 => db.debug;
"../../PitchTrack/data/obama.wav" => db.read;

<<<"word scale, grainPeriod .35sec">>>;
"plancktaper95" => db.grainWindow;
49.6 => float start;
.35::second => db.grainPeriod; // seconds per grain 
1::second / (.5*db.grainPeriod()) => db.triggerFreq;  // 2 triggers per grain dur
start => db.phasorStartSec;
1 => db.grainRate;
5::second => now;

<<<"word scale, grainRand 1 (randomized grainDur)">>>;
1 => db.debug;
1 => db.grainRand;
1 => db.grainRandFreq;
start => db.phasorStartSec;
5::second => now;

<<<"word scale, smaller grains, more&faster randomness">>>;
"blackman" => db.grainWindow;
0 => db.debug;
1 => db.gain;
1 => db.grainRand;
10 => db.grainRandFreq;
start => db.phasorStartSec;
start + .5 => db.phasorStopSec;
1::ms => db.grainPeriod;
800 => db.triggerFreq;
5::second => now;

