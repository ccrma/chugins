# ConvRev: A Convolution Reverb Chugin

## m220A Final Project -- Summer 2021. Edited and improved Fall 2023

### Build Options

The ConvRev `makefile` defines a flag `FFTCONVOLVER_USE_SSE`. This tells the convolution engine to use SIMD vector intrinsics, which by my tests increases performance up to 2x.
To enable SSE support for MacOS, ConvRev uses the [sse2neon](https://github.com/DLTcollab/sse2neon/tree/master) translation header.

If your OS does not support either SSE or Neon, comment out the `FFTCONVOLVER_USE_SSE` flag before building.

#### HOWTO Build on Mac

In the `ConvRev/` directory, run

```
make osx
sudo make install
```

#### HOWTO Use the ConvRev Ugen

First, load an impulse response of your choosing into a SndBuf

```
SndBuf2 ir => blackhole;
"../IRs/hagia_sophia.wav" => ir.read;
```

Then seed the ConvRev ugen with this IR. Unfortunately the current method is clunky, let me know if there's a cleaner way to do this. (Ideally we could load the wav file directly into ConvRev without needing SndBuf at all, but I'm not sure how to do that in a Chugin)

```
ConvRev cr;
ir.samples() => int order;
order => cr.order;  // set the IR length
for (0 => int i; i < order; i++) {
  cr.coeff(i, ir.valueAt(i));  // set each IR sample value
}
```

Initialize the ConvRev ugen. Most importantly this allocates the memory according to the IR size specified in the previous step. The default blocksize is 128.

```
256 => cr.blocksize; // FFT size, set to any power of 2
cr.init();  // initialize the ConvRev engine
```

Finally, connect to dac and make music!

```
adc => cr => dac;  // or whatever you wish...
```

The ConvRev ugen is designed to run in real time, with a fixed delay of 2*blocksize (this time is necessary to buffer that many samples and then perform convolution). For the default blocksize of 128 samples, the delay at 44khz is 5.8ms.

If the delay is a problem, you can either decrease the blocksize (which may adversely effect performance) OR run your dry signal through a delay ugen of equal duration.

In the near future I may be optimizing the convolution algorithm to have negligible delay.

#### Sources Cited

The overlap-add convolution implementation is taken from the [HiFi-LoFi FFTConvolver Library](https://github.com/HiFi-LoFi/FFTConvolver), under the MIT license.
