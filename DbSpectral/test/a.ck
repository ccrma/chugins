Noise snd => DbSpectral filt => dac;
SqrOsc sqr => filt;
snd.gain(.3);
sqr.gain(.3);

<<<"through filter with no image (fft->ifft)">>>;
5::second => now;

<<<"bypass">>>;
filt.op(-1); // bypass
5::second => now;
