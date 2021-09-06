// run: chuck _pre.ck str_pitchbend.ck
// if a string starts with "X:#", we can pass it to DbAbc::open.
// abc requires that the header portion terminate with K:<key>.
"X:1\n" +
"M:4/4\n" +
"Q:1/4=80\n"+
"K:C\n"+
"|:!bend!G AAA |!bend!C FFF:|\n" +
"%%MIDI bendstring 1000 1000 -500 -550\n" +
"|:!bend!G AAA |!bend!C FFF:|\n"
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

