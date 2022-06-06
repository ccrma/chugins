// validate image-changes.
DbSpectral filt => dac;
Noise noise => filt;
SqrOsc sqr => filt;
noise.gain(.7);
sqr.gain(.3);
filt.gain(.5);
for(int h;h<2;h++)
{
    "image" + h + ".png" => string inm;
    <<<"loading", inm>>>;
    filt.loadSpectogram(me.dir() + inm); // async
    for(int i;i<10;i++)
    {
        Math.random2(10, 200) => int scanRate;
        <<<"ScanRate", scanRate>>>;
        filt.scanRate( scanRate );
        for(int j;j<20;j++)
        {
            .1::second => now;
            Math.mtof(Math.random2(40, 60)) => sqr.freq;
        }
    }
}

