SndBuf snd => DbSpectral filt => dac;
snd.gain(.5);
snd.loop(1);
snd.read("../../PitchTrack/data/obama.wav");

snd.phase(0);
10::second => now;

snd.phase(0);
filt.op(-1); // bypass
10::second => now;