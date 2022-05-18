// ontherun without any of the good stuff.
// (just a demo of DbSpectral, EQ only)
DbSpectral filt => dac;
BlitSquare sqr => filt;
sqr.gain(.3);
filt.gain(1);
filt.loadSpectogram(me.dir() + "image1.png"); // async
filt.scanRate(30);
[52, 55, 57, 55, 62, 60, 62, 64] @=> int notes[];
-5 => int transpose;
160 => float bpm;
(60/bpm) * 1::second => dur beatDur;
beatDur * .25 => dur noteDur;
while(true)
{
    for(int j;j<20;j++)
    {
        for(int k;k<notes.size();k++)
        {
            Math.mtof(notes[k] + transpose) => sqr.freq;
            noteDur => now;
        }
    }
}

