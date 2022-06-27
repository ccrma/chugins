// compare fft->ifft (no EQ, etc) with bypass.
SndBuf snd => DbSpectral filt => dac;
snd.gain(.5);
snd.loop(1);
snd.read(me.dir() + "../../PitchTrack/data/obama.wav");

<<<"plain signal, no image">>>;
snd.phase(0);
10::second => now;

<<<"bypass">>>;
snd.phase(0);
filt.op(-1); // bypass
10::second => now;