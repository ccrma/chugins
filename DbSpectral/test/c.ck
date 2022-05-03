
DbSpectral filt => dac;
Noise noise => filt;
SqrOsc sqr => filt;
noise.gain(.25);
sqr.gain(.25);

filt.loadImage(me.dir() + "img0.png"); // async
filt.gain(2);

for(int i;i<20;i++)
{
    filt.scanRate( Math.random2(100, 200) );
    for(int j;j<10;j++)
    {
        .2::second => now;
        Math.mtof(Math.random2(40, 60)) => sqr.freq;
    }
}

