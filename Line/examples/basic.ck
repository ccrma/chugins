@import "Line"

// Line can be used as a replacement for Envelope.

SinOsc s(880) => Line l(5::second) => dac;

// Equivalent to `Line l(5::second)`
// l.set(5::second);

// Activates the ramp, and advances time until the ramp is finished.
l.keyOn() => now;

// keyOff ramps back down to 0
l.keyOff(1::second) => now;

1::second => now;

// alternatively you set the ramp duration and trigger the ramp at the same time
l.keyOn(5::second) => now;

l.keyOff(1::second) => now;