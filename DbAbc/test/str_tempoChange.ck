// run: chuck _pre.ck str_tempoChange.ck
// if a string starts with "X:#", we can pass it to DbAbc::open.
// abc requires that the header portion terminate with K:<key>.
"X:1\n" +
"M:4/4\n" +
"Q:1/4=120\n"+
"K:G\n" +
"V:1\n" +
"P:A\n"+
"|: G>A BB cBAc|BAGF E2D2|G>A BB cBAc|BGAF G2 G2\n"+
"P:B\n"+
":: d c/2B/2 AB c B/2A/2 GB|AGFG A>B A2|\\\n"+
"   d c/2B/2 AB c B/2A/2 GB|AGGF G2 G2 ::\n"+
"P:C\n"+
"K:F\n"+
"B/2A/2 G B/2A/2 G FG A2|DEFG ABAG | \\\n"+
"B/2A/2 G B/2A/2 G FG A2|DEFG GF G2 :|\n"  +
"V:2\n%voice to issue global tempo changes\n"+
"|Z8|\nQ:1/4=100\n|Z8|\nQ:1/4=200\n"
=> string tune;

DoTrack doit;
DbAbc dbf;
// dbf.configure(["-v", "6"]);

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
