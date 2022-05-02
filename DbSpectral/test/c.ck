
SndBuf snd => DbSpectral filt => dac;

Noise noise => filt;
noise.gain(.1);

SqrOsc sqr => filt;
sqr.gain(.25);

snd.gain(.1);
snd.loop(1);
snd.read("../../PitchTrack/data/obama.wav");

filt.loadImage(me.dir() + "about.png"); // async
filt.gain(2);

snd.phase(0);
30::second => now;

