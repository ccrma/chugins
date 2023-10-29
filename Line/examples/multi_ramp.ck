// Line can be used to generate a sequence of ramps

SawOsc s => Line l => dac;

// Create a ramp that goes from 0 to 1 in 1::second
l.set([0.1, 0.0, 0.5, 0.1, 1.0], [1::second, 1::second, 0.25::second, 0.25::second, 0.125::second]);

// Activates the ramp, and advances time until the ramp is finished.
l.keyOn() => now;

// keyOff ramps back down to 0
l.keyOff(1::second) => now;

1::second => now;

// alternatively you set the ramp duration and trigger the ramp at the same time
l.keyOn([0.1, 0.0, 0.5, 0.1, 1.0], [1::second, 0.5::second, 0.25::second, 0.25::second, 0.125::second]) => now;

l.keyOff(1::second) => now;