<<<"Hello from DbGrain">>>;

// DbGrainBuf db => NRev rev => dac;
DbGrainBuf db => dac;
0 => db.debug;
"../../PitchTrack/data/obama.wav" => db.read;
db.filelen() => dur filelen;
<<<"filelen:", filelen / 1::second, "seconds">>>;

<<<"repeating grains (2.5sec trigger, 2.5sec grain, so repeating)">>>;
.5 => db.gain; // lots of overlap
10::second => db.grainPeriod; 
// "blackman" => db.grainWindow;
"plancktaper" => db.grainWindow;
.35 => db.phasorStart; // phasor is a percentage
.35 + 2.5::second / filelen => db.phasorStop; 

1/2.5 => db.triggerFreq; // one trigger every 2.5 sec
15::second => now;

.05 => db.phasorStop; //  should generate error
1::second => now;

