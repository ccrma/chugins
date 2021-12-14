FGridMsg msg; 
DbFGrid f;
me.dir() + "../samples/oneOctave.fgrd" => string file;
f.open(file);

int instUsage[128];

6 => int nvoices;
Wurley c[6];
for(int i;i<nvoices;i++)
{
    c[i] => dac;
    .3 => c[i].gain;
}

0 => int inst;
1 => float timeScale;

for(int i;i<20;i++)
{
    while(true)
    {
        if(f.read(msg) == 0)
        {
            if(msg.type == 0)
                timeScale * msg.value::second => now;
            else
            if(msg.type == 1)
            {
                msg.note $ int => int inote;
                if(i == 0)
                    <<< "down", inote, msg.value >>>;
                Math.mtof(msg.note) => c[inst].freq;
                c[inst].noteOn(msg.value);
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
                c[ninst].noteOff(1);
            }
        }
        else
            break;
    }
    f.rewind();
    if(i < 12)
        .8 *=> timeScale;
}
