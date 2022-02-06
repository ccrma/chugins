DbFGrid db;
"wavetable.fgrd" => string file;

db.open(file) => int err;

<<<"open returned", err>>>;

int npts;
float x[0];
db.readChannel("Wavetable", 8/*col*/, 107/*row*/, 0/*layer*/, x) => npts;
<<<"readTable read", npts, "points">>>;
if(npts != 0)
{
    <<<"table[random]", x[Math.random2(0, npts-1)]>>>;
}

// rescale 0-1 onto -1, 1
for(int i;i<x.size();i++)
{
    2 * x[i] - 1 => x[i];
}

Phasor p => Wavetable tab => dac;
1 => tab.op;
2 => tab.interpolate;
x => tab.setTable;
800 => p.freq;

5::second => now;

    

