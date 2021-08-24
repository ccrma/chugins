// to run: 
//      chuck _pre.ck e.ck
DoTrack doit;
DbAbcFile dbf;

// demo4 - 4 part arrangement
"../samples/demo4.abc" => string file;
file => dbf.open;
<<< file, "nchan", dbf.numTracks() >>>;

[0] @=> int running[];
1 => float timeScale;
for(0=>int t; t < dbf.numTracks(); t++)
{
    1 +=> running[0];
    spork ~ doit.go(dbf, t, timeScale, running); // t==1 ? s : f);
}
while(running[0] > 0)
    1::second => now;
dbf.close();
<<<"ring">>>;
2::second => now;

