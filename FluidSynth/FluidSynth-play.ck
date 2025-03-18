@import "FluidSynth"

NRev rev => dac;
0.01 => rev.mix;

MidiFileIn min;
MidiMsg msg;

me.dir() + "HS_African_Percussion.sf2" => string sfont;
if(me.args() > 0) me.arg(0) => sfont;

me.sourceDir() + "/bwv772.mid" => string filename;
if(me.args() > 1) me.arg(1) => filename;

if(!min.open(filename))
{
    cherr <= "unable to open MIDI file: '" <= filename <= "'\n";
    me.exit();
}

chout <= filename <= ": " <= min.numTracks() <= " tracks\n";

FluidSynth m => rev;
m => dac;
0.91 => m.gain;
m.open(sfont);

int done;

for(int t; t < min.numTracks(); t++)
    spork ~ track(t);

while(done < min.numTracks())
    1::second => now;

min.close();

minute => now;

fun void track(int t)
{
    
    
    while(min.read(msg, t))
    {
        if(msg.when > 0::second)
            msg.when => now;
        
        if((msg.data1 & 0x90) == 0x90)
            m.noteOn(msg.data2, msg.data3, msg.data1&0x0F);
        //if((msg.data1 & 0x80) == 0x80)
        //    m.noteOff(msg.data2, msg.data1&0x0F);
    }
    
    done++;
}
