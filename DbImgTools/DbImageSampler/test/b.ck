// "On The Run" without any of the good stuff.
BPF filt => dac;
BlitSquare sqr => filt;
sqr.gain(.8);

fun float remap(float v, float min, float max)
{
    return min + (max - min) * v;
}
fun void modFilter()
{
    DbImageSampler sampler;
    sampler.loadImage(me.dir() + "aulacoctena.png"); // async

    .1 => float aFreq; 
    .9 => float aAmp; 
    pi/2 => float aPhase;
    .05 => float bFreq; 
    .1 => float bAmp;
    50::ms => dur samplePeriod;

    float t;
    2*pi*samplePeriod/1::second => float tstep;
    while(true)
    {
        samplePeriod => now; // 50fps
        // sample from a lissajous curve 
        aAmp * Math.sin(aFreq * t + aPhase) => float x;
        bAmp * Math.sin(bFreq * t) => float y;
        .5 * (x + 1) => x;
        .5 * (y + 1) => y;
        sampler.getSample(x, y) => vec4 s;

        remap(s.r, 4, 8) => float q;
        filt.Q(q);
        remap(s.g, 300, 3000) => float f;
        filt.freq(f);
        // <<<"freq", f, "Q", q>>>;
        tstep +=> t;
    }
}

spork ~ modFilter();

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