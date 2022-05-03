# DbSpectral Chugin

## Intro

> DbSpectral is a time-varying audio filter defined by
> an image file.  DbSpectral reads image files (.png) representing a 
> [spectrogram](https://en.wikipedia.org/wiki/Spectrogram).
> The image y-axis is interpretted as uniformly sampled frequency from 
> high (Nyquist) at y=0 to low (DC) at y=imageHeight. The image x-axis 
> is interpretted as time and you can define a _scanning rate_ 
> (measured in pixels/second) to control the rate at which the current
> filter weights are updated.
>
> alt.intro:
> * imagine turning a [graphic equalizer](https://en.wikipedia.org/wiki/Equalization_(audio)#Graphic_equalizer)
>   on its side.
> * now "stack" a bunch (10, 100, 1000) of equalizers next to each other with 
>   different settings for each segment of your composition.
> * a pixel's brightness represents the equalizer's slider position.
> * take out your favorite paint package (or use Fiddle's built-in one)
>   and start painting or drawing patterns that represent the filter changes.

## Details

We compute the FFT of the incoming signal and multiply the frequency-space
result by the values in the current column of your image. The result is
converted back to the time-domain and sent to the filter output. When loaded,
your image is resized so that the vertical dimensioni matches the size of 
the FFT.  

For example, a RealFFT of size 1024 produces 512 frequency bins for both 
real and imaginary components.  The normalized pixel value at [x, y] 
is multiplied by both components of the FFT prior to performing the IFFT.
It would be quite easy to support a two-channel image specification of these
weights, but for now, we've haven't found sufficient justification.

Files are loaded in a separate thread to prevent hiccups in the
audio/realtime thread.  This also affords us the opportunity to load new
images during a live performance.

The FFT/IFFT is also computed in a separate thread so as not to bog down the
realtime audio thread.  We apply Hann windowing to the signal (pre and post 
transform) and have an overlap of 25%.

Currently we "point-sample" the column data. If this presents a problem
image-scale your input in x and modify `scanRate` accordingly.

## Example

```
// "On The Run" without any of the good stuff.
DbSpectral filt => dac;
BlitSquare sqr => filt;
sqr.gain(.3);
filt.gain(1);
filt.loadImage(me.dir() + "image1.png"); // async
filt.scanRate(30); // 34.1sec per loop (30 cols/sec, w:1024)
[52, 55, 57, 55, 62, 60, 62, 64] @=> int notes[];
-5 => int transpose;
160 => float bpm;
(60/bpm) * 1::second => dur beatDur;
beatDur * .25 => dur noteDur;
while(true)
{
    for(int j;j<20;j++)
    {
        for(int k;k<notes.size();k++)
        {
            Math.mtof(notes[k] + transpose) => sqr.freq;
            noteDur => now;
        }
    }
}
```

## API

| Method                                  | Description                                                                                                                |
| :-------------------------------------- | :------------------------------------------------------------------------------------------------------------------------- |
| `int fftSize(int fftsz, int overlapsz)` | Requests an FFT size and overlap, default is (512,128). Can't be changed during performance. FFT size must be a power of 2. |
| `loadImage(string path)`                | Loads the Spectrogram asynchronously.                                                                                      |
| `scanRate(int columnsPerSecond)`        | Requests a scanning rate.                                                                                                  |
| `int getColumn()`                       | Returns the current scanning column.                                                                                       |

