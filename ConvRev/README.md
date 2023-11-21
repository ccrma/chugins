# ConvRev: A Convolution Reverb Chugin

## m220A Final Project -- Summer 2021

#### ConvRev.zip Contents

- __chugin_includes__: Includes to compile chugins. Copied from the official chugin git repository, no need to touch.
- __convolved__: Examples wav files convolved with the ConvRev chugin
- __ConvRev__: chugin source code + makefile. Build instructions below.
- __examples__: chuck examples for how to use ConvRev with SndBuf or ADC input
- __IRs__: a few impulse responses to seed the ConvRev ugen
- TODO: add presentation

#### Build Requirements

This Chugin performs FFTs using the [Apple Accelerate Framework dsp library](https://developer.apple.com/documentation/accelerate/vdsp), and so it is only compatible with mac os. Developer tools may need to be installed.

Future work includes switching the FFT implementation to the built-in chuck version, which would make this Chugin buildable on any OS.


#### HOWTO Build

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

PLEASE NOTE that the overlap-add convolution implementation is taken from the [HiFi-LoFi FFTConvolver Library](https://github.com/HiFi-LoFi/FFTConvolver), under the MIT license. All cpp and header files under the `ConvRev/` directory, excluding `ConvRev.cpp`, are from this library.
