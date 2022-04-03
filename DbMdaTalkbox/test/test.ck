DbMdaTalkbox talk => dac;
talk.printParams();

Pan2 pan; 
SndBuf buf;
SqrOsc sqr;
buf => pan.left;
sqr => pan.right;
sqr.gain(.5);

pan => talk;

buf.loop(1);
buf.read("../../PitchTrack/data/obama.wav");
buf.pos(0);

0.1 => float dry;
talk.setParam(1, dry);
<<<"dry", dry>>>;
for(1=>int j;j<9;j++)
{
    j/10. => float wet; 
    talk.setParam(0, wet);
    <<<"wet", wet>>>;
    for(int i;i<10;i++)
    {
        Math.random2f(50, 200) => sqr.freq;
        .5::second => now;
    }
}
