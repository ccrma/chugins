# DbImageSampler Chugin

## Overview 

Images are 2D vector fields.  At each (x, y) position there are
3 or 4 values (RGB, RGBA) representing the color of a pixel.
Beyond coloring a pixel on your screen, these values can be 
used to parameterize any ol' thing.  Consider, for example, a 
digital filter.  Two numbers are required to express the filter 
quality and frequency threshold.  We can "automate" filter sweeps 
painting parameter values into an image, then animating the x, y 
lookup coordinates.

`DbImageSampler` provides access to the pixel values in your image files. 
Once an image is loaded, you sample the image using normalized image 
coordinates (x, y between 0 and 1).  The result of the sampling process is 
an RGBA value that you can use for your own creative/nefarious purposes. 


## API

| Method                             | Description                                                                                                                         |
| :--------------------------------- | :---------------------------------------------------------------------------------------------------------------------------------- |
| `loadImage(string path)`           | Loads an image asynchronously.                                                                                                      |
| `vec4 getSample(float x, float y)` | Requests a sample from the current image. Results are place into a vec4. Channels are [0-1] and accessed via `.r`, `.g`, `.b`, `.a` |


## Example

```
/// "On The Run" without any of the good stuff.
BPF filt => dac;
BlitSquare sqr => filt;
sqr.gain(.8);

fun float remap(float v, float min, float max)
{
    return min + (max - min) * v;
}
fun void modFilter()
{
    DbImageSampler sampler;
    sampler.loadImage(me.dir() + "aulacoctena.png"); // async

    .1 => float aFreq; 
    .9 => float aAmp; 
    pi/2 => float aPhase;
    .05 => float bFreq; 
    .1 => float bAmp;
    50::ms => dur samplePeriod;

    float t;
    2*pi*samplePeriod/1::second => float tstep;
    while(true)
    {
        samplePeriod => now; // 50fps
        // sample from a lissajous curve 
        aAmp * Math.sin(aFreq * t + aPhase) => float x;
        bAmp * Math.sin(bFreq * t) => float y;
        .5 * (x + 1) => x;
        .5 * (y + 1) => y;
        sampler.getSample(x, y) => vec4 s;

        remap(s.r, 4, 8) => float q;
        filt.Q(q);
        remap(s.g, 300, 3000) => float f;
        filt.freq(f);
        // <<<"freq", f, "Q", q>>>;
        tstep +=> t;
    }
}

spork ~ modFilter();

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

## Credits

DbImageSampler is built atop this open-source component:

* [stb_image](http://nothings.org/stb)

## License

DbImageSampler is open-source.  Sub-components are subject to their own
licenses.  Unless otherwise stated, this code is subject to the
MIT license:

Copyright 2022 Dana Batali

Permission is hereby granted, free of charge, to any person obtaining a copy 
of this software and associated documentation files (the "Software"), to 
deal in the Software without restriction, including without limitation the 
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
sell copies of the Software, and to permit persons to whom the Software is 
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in 
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.