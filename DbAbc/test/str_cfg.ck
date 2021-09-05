//
// passing configuration args to DbAbc
// to run: chuck _pre.ck str_cfg.ck
// 
DbAbc abc;
DoTrack doit;

["-v", "6"] => abc.configure;
"X:1\nM:4/4\nK:C\n|CDEF GABc|\n" => abc.open;

[0] @=> int running[];
1 => float timeScale;
for(0=>int t; t < abc.numTracks(); t++)
{
    1 +=> running[0];
    spork ~ doit.go(abc, t, timeScale, running); // t==1 ? s : f);
}
while(running[0] > 0)
    1::second => now;
abc.close();
<<<"ring">>>;
2::second => now;