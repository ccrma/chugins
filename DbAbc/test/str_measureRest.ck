// run: chuck _pre.ck str_twovoiceRest.ck
// this example shows how multi-measure rests work (V:3).
"X:1\n" +
"M:4/4\n"+
"K:G\n"+
"P:A\n"+
"V:1\n"+
"|:CzCz CzCz|Z4:|\n"+
"V:2\n"+
"|:zGzG zGzG|Z2:|\n" +
"V:3\n"+
"|:Z10|zzCC zzCC:|\n"
=> string tune;

DoTrack doit;
DbAbc dbf;

tune => dbf.open => int success;
if(success == 0)
{
    <<<"problem with abc string">>>;
    me.exit();
}

<<< "abcstring", "nchan", dbf.numTracks() >>>;

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

