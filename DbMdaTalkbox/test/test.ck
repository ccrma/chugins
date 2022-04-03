Pan2 pan => DbMdaTalkbox talk => dac;
talk.printParams();

SqrOsc sqr;
sqr => pan.right; // carrier
sqr.gain(.5);

SndBuf buf;
buf => pan.left;  // modulator
buf.gain(.5);
buf.loop(1);
buf.read("../../PitchTrack/data/obama.wav");
buf.pos(0);

0.1 => float dry;
talk.setParam(1, dry);
<<<"dry", dry>>>;
for(1=>int j;j<=10;j++)
{
    j/10. => float wet; 
    talk.setParam(0, wet);
    <<<"wet", wet>>>;
    for(int i;i<10;i++)
    {
        Math.random2f(50, 200) => sqr.freq;
        .60::second => now;
    }
}
