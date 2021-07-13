DbGrainBuf db => dac;
"../kick.wav" => db.read;

1 => db.loop;
5::second => now;

-1 => db.rate;
5::second => now;

4 => db.rate;
5::second => now;

-4 => db.rate;
5::second => now;

8 => db.rate;
5::second => now;

<<<"maxfilt", db.maxfilt()>>>;
16 => db.rate;
5::second => now;

32 => db.maxfilt;
16 => db.rate;
5::second => now;

2 => db.maxfilt;
16 => db.rate;
5::second => now;

