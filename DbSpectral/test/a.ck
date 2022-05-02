Noise snd => DbSpectral filt => dac;
snd.gain(.5);

<<<"filtered">>>;
5::second => now;

<<<"bypass">>>;
filt.op(-1); // bypass
5::second => now;
