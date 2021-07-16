<<<"Hello from DbGrain">>>;

// DbGrainBuf db => NRev rev => dac;
DbGrainBuf db => dac;
.5 => db.gain;

0 => db.debug;
"../../PitchTrack/data/obama.wav" => db.read;

<<<"'scale of our', grainPeriod .35sec">>>;
"plancktaper95" => db.grainWindow;
49.6 => float start;
1 => db.grainRate;
.35::second => db.grainPeriod; // seconds per grain 
1::second / (.5*db.grainPeriod()) => db.triggerFreq;  // 2 triggers per grain dur
start => db.phasorStartSec;
start + .5 => db.phasorStopSec;
5::second => now;

<<<"phasorRate 2 (proceed through file double speed - but same trigger)">>>;
2 => db.phasorRate;
5::second => now;

<<<"wobble .5, default wobbleFreq">>>;
.5 => db.phasorWobble;
5::second => now;

<<<"wobble 2, wobbleFreq .5">>>;
2 => db.phasorWobble;
.5 => db.phasorWobbleFreq;
5::second => now;

<<<"wobbleFreq 10">>>;
10 => db.phasorWobbleFreq;
5::second => now;

<<<"wobbleFreq 1">>>;
1 => db.phasorWobbleFreq;
5::second => now;

<<<"wobble .5">>>;
.5 => db.phasorWobble;
5::second => now;

<<<"wobble 0, zero-phase">>>;
0 => db.phasorWobble;
start => db.phasorStartSec;
start => db.phasorStopSec;
5::second => now;

<<<"wobble 5000, zero-phase">>>;
5000 => db.phasorWobble;
start => db.phasorStartSec;
start => db.phasorStopSec;
5::second => now;

<<<"wobble 5000, zero-phase, 4Hz">>>;
10000 => db.phasorWobble;
4 => db.phasorWobbleFreq;
5::second => now;



