# Range (unit generator)
  |- (inheritance) Range -> UGen -> Object


  `Range` is a UGen that maps an input range to an output range. It currently supports linear mapping between ranges as well as clipping.
  This is particularly useful for LFOs where the useful parameter values don't center around one.

  You can define ranges in terms of min/max values:

  ```
  // Use scaler to make an easy vibrato
SinOsc lfo => Range r => blackhole;
SinOsc tone => dac;

3 => lfo.freq;

// Modulate frequency +- 10Hz around A440
(-1, 1, 430, 450) => r.range 

while(10::ms => now) {
	r.last() => tone.freq;
}
```

...or by defining a range center value and a radius size:

```
// Use scaler to make an easy vibrato
SinOsc lfo => Range r => blackhole;
SinOsc tone => dac;

// Help reveals all secrets
Range.help();

3 => lfo.freq;
// Modulate frequency +- 10Hz around A440
(0, 1, 440, 10) => r.radius;

while(10::ms => now) {
	r.last() => tone.freq;
}
```

## Range Functions

* `int clip( int val );`

    Set Range's clipping mode. Accepted values are: 0 (no clipping), 1 (hard clipping). Defaults to 0
* `int clip();`

    Get clip state.
* `float inCenter();`

    Get input center.
* `float inMax();`

    Get input max.
* `float inMin();`

    Get input min.
* `float inRad();`

    Get input radius.
* `void inRadius( float center, float radius );`

    Set the expected input radius.
* `void inRange( float min, float max );`

    Set the expected input range.
* `float outCenter();`

    Get output center.
* `float outMax();`

    Get output max.
* `float outMin();`

    Get output min.
* `float outRad();`

    Get output radius.
* `void outRadius( float center, float radius );`

    Set the output radius.
* `void outRange( float min, float max );`

    Set the output range.
* `void radius( float inCenter, float inRadius, float outCenter, float outRadius );`

    Set the expected input range and desired output range in terms of a center with a radius.
* `void range( float inMin, float inMax, float outMin, float outMax );`

    Set the expected input range and desired output range.
