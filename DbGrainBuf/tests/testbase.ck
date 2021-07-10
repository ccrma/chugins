<<<"Hello from DbGrain">>>;

DbGrainBuf db => NRev rev => dac;

"../../PitchTrack/data/obama.wav" => db.read;

<<<"Rate 1">>>;
5::second => now;

<<<"Rate 2">>>;
2 => db.rate;
5::second => now;

<<<"Rate .5">>>;
.5 => db.rate;
10::second => now;

<<<"Rate 10">>>;
10 => db.rate;
10::second => now;

<<<"Loop on, reset">>>;
1 => db.loop;
0 => db.pos;
10::second => now;

