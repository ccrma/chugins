<<<"Hello from DbGrain">>>;

DbGrainBuf db => NRev rev => dac;
.5 => db.gain;
.5 => rev.mix;
1 => db.bypass;

"../../PitchTrack/data/obama.wav" => db.read;

//.5 => db.rate;
//(2.775*40000) $ int => db.pos;
//.85::second => now;

.5 => db.rate;
10::second => now;

// .25 => db.rate;
// (2.850*40000) $ int => db.pos;
// 1.55::second => now;

