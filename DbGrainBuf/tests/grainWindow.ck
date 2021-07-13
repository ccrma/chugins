<<<"Hello from DbGrain">>>;

// DbGrainBuf db => NRev rev => dac;
DbGrainBuf db => dac;
"../../PitchTrack/data/obama.wav" => db.read;

<<<"One grain to evaluate windowing (soft->loud->soft)">>>;
.1 => db.triggerFreq; // one trigger every 10 sec
10 => db.grainPeriod; // seconds
10::second => now;

<<<"Four live grains (gets louder due to overlap, ie: long phase)">>>;
0 => db.phasorStart;
1/2.5 => db.triggerFreq; // one trigger every 2.5 sec
10 => db.grainPeriod; // seconds for each grain
10::second => now;

<<<"Four live repeating grains (5sec phase, so repeat)">>>;
db.filelen() => float filelen; // returns filesize in seconds
0 => db.phasorStart;
5. / filelen => db.phasorStop;
1/2.5 => db.triggerFreq; // one trigger every 2.5 sec
10 => db.grainPeriod; // seconds for each grain
10::second => now;

