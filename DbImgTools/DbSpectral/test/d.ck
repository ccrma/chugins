// A demo of DbSpectral, Mode 1: EQ+Delay
ModalBar sqr => DbSpectral filt => dac;
sqr.preset(1);
filt.gain(4);
filt.init(2048/*fft*/, 512/*overlap*/, 1/*EQ+Delay*/);
filt.freqMin(40);
filt.freqMax(4000);
filt.delayMax(3.); // seconds
filt.mix(.95);
filt.loadSpectogram(me.dir() + "delay.png"); // async
filt.feedbackMax(.1);
[52, 55, 57, 55, 62, 60, 62, 64] @=> int notes[];
0 => int transpose;
30 => float bpm;
(60/bpm) * 1::second => dur beatDur;
beatDur * .25 => dur noteDur;

filt.scanRate((1024*2) / notes.size()); // 

while(true)
{
    for(int j;j<20;j++)
    {
        Math.random2(-2, 2) => transpose;
        for(int k;k<notes.size();k++)
        {
            Math.mtof(notes[k] + transpose) => sqr.freq;
            sqr.strike(Math.random2f(.5, 1.));
            Math.random2(1, 5) * noteDur => now;
        }
    }
}

