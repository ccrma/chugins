// Line can be used as a replacement for Envelope.

SinOsc s => Line l => dac;

// Create a ramp that goes from 0 to 1 in 1::second
l.set(1::second);

// Activates the ramp, and advances time until the ramp is finished.
l.keyOn() => now;

// keyOff ramps back down to 0
l.keyOff(1::second) => now;

1::second => now;

// alternatively you set the ramp duration and trigger the ramp at the same time
l.keyOn(1::second) => now;

l.keyOff(1::second) => now;