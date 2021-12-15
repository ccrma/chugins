FGridMsg msg; 
DbFGrid f;
me.dir() + "../samples/oneOctave.fgrd" => string file;
f.open(file);

<<< "file has", 
    f.numLayers(), "layers", 
    "bar size:", f.barSize(),
    "beat unit:", f.beatSize()>>>;

int instUsage[128];

6 => int nvoices;
Wurley c[6];
BeeThree d[6];
for(int i;i<nvoices;i++)
{
    c[i] => dac;
    d[i] => dac;
    .3 => c[i].gain;
    .3 => d[i].gain;
}

0 => int inst;
1 => float timeScale;

for(int i;i<20;i++)
{
    while(true)
    {
        if(f.read(msg, -1/*all layers*/) == 0)
        {
            if(msg.type == 0)
                timeScale * msg.value::second => now;
            else
            if(msg.type == 1)
            {
                msg.note $ int => int inote;
                if(i == 0)
                    <<< "down", inote, msg.value >>>;
                Math.mtof(msg.note) => float freq;
                if(msg.layer == 0)
                {
                    c[inst].noteOn(msg.value);
                    freq => c[inst].freq;
                }
                else
                {
                    d[inst].noteOn(msg.value);
                    freq => d[inst].freq;
                }
                inst => instUsage[inote];
                (inst + 1) % nvoices => inst;
            }
            else
            if(msg.type == 2)
            {
                msg.note $ int => int inote;
                if(i == 0)
                    <<< "up", inote>>>;
                instUsage[inote] => int ninst;
                if(msg.layer == 0)
                    c[ninst].noteOff(1);
                else
                    d[ninst].noteOff(1);
            }
        }
        else
            break;
    }
    f.rewind();
    if(i < 12)
        .8 *=> timeScale;
}
