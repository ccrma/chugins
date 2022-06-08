// EQ-image
DbSpectral filt => dac;
Noise noise => filt;
SqrOsc sqr => filt;
noise.gain(.2);
sqr.gain(.2);

filt.loadSpectogram(me.dir() + "image0.png"); // async
filt.gain(1.5);

for(int i;i<20;i++)
{
    filt.scanRate( Math.random2(100, 200) );
    for(int j;j<10;j++)
    {
        .1::second => now;
        Math.mtof(Math.random2(40, 60)) => sqr.freq;
    }
}

