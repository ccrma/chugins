DbGrainBuf db => dac;
"../kick.wav" => db.read;
1 => db.debug;
1 => db.bypass;

<<<"fwd 1">>>;
1 => db.loop;
5::second => now;

<<<"rev 1">>>;
-1 => db.rate;
5::second => now;

<<<"fwd 4">>>;
4 => db.rate;
5::second => now;

<<<"rev 4">>>;
-4 => db.rate;
5::second => now;

<<<"fwd 8">>>;
8 => db.rate;
5::second => now;

16 => db.rate;
<<<"fwd 16 maxfilt:", db.maxfilt()>>>;
5::second => now;

<<<"fwd 16">>>;
32 => db.maxfilt;
<<<"fwd 16 maxfilt:", db.maxfilt()>>>;
16 => db.rate;
5::second => now;

2 => db.maxfilt;
<<<"fwd 16 maxfilt:", db.maxfilt()>>>;
16 => db.rate;
5::second => now;

