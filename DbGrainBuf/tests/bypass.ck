<<<"Hello from DbGrain">>>;

// DbGrainBuf db => NRev rev => dac;
DbGrainBuf db => dac;
1 => db.bypass;
"../../PitchTrack/data/obama.wav" => db.read;
db.printinfo();

<<<"GrainBuf bypass">>>;
10::second => now;

