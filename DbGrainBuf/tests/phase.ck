DbGrainBuf db => NRev rev => dac;
1 => db.bypass;
1.25 => db.rate;
"../../PitchTrack/data/obama.wav" => db.read;
.95 => db.phase; // start 95% through the file
1 => db.loop;
7::second => now;

