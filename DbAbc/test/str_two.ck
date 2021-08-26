// run: chuck _pre.ck str_two.ck
// if a string starts with "X:#", we can pass it to DbAbc::open.
// abc requires that the header portion terminate with K:<key>.
"X:1\n"+
"M:4/4\n"+
"Q:1/4=200\n"+
"K:C\n"+
"|:CDEF GABc|BAGF EDCC:|\n" +
"K:D\n"+
"|:CDEF GABc|BAGF EDCC:|\n" +
"K:C\n"+
"|:CDEF GABc|BAGF EDCC:|\n"
=> string tune1;

"X:2\n" +
"M:4/4\n" +
"Q:1/4=200\n" +
"K:C transpose=7\n" +
"|:CDEF GABc|BAGF EDCC:|\n" +
"K:D\n" +
"|:CDEF GABc|BAGF EDCC:|\n" +
"K:C\n" +
"|:CDEF GABc|BAGF EDCC:|\n"
=> string tune2;

// "|GABc defg|fedc BAGG:|\n" => string tune2;

DoTrack doit1;
DoTrack doit2;
DbAbc dbf1;
DbAbc dbf2;

tune1 => dbf1.open => int success;
if(success == 0)
{
    <<<"problem with abc string">>>;
    me.exit();
}

tune2 => dbf2.open => success;
if(success == 0)
{
    <<<"problem with abc string">>>;
    me.exit();
}

[0] @=> int running[];
1 => float timeScale;
for(0=>int t; t < dbf1.numTracks(); t++)
{
    1 +=> running[0];
    spork ~ doit1.go(dbf1, t, timeScale, running); // t==1 ? s : f);
}

for(0=>int t; t < dbf2.numTracks(); t++)
{
    1 +=> running[0];
    spork ~ doit2.go(dbf2, t, timeScale, running); // t==1 ? s : f);
}

while(running[0] > 0)
    1::second => now;
dbf1.close();
dbf2.close();
<<<"ring">>>;
2::second => now;
