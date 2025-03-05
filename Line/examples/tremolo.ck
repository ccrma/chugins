@import "Line"

TriOsc osc(880) => Gain g => dac;
SinOsc lfo(5) => Patch p(g, "gain") => blackhole;

Step s =>
Line l(5, [1.0, 2, 5], [5::second, 0.5::second, 0.1::second]) =>
Patch t(lfo, "freq") => blackhole;

// print out lfo's frequency
spork~ print();

// run forever
while (l.keyOn() => now);

fun print() {
    while (true) {
          <<< "lfo.freq:", l.last() >>>;
          100::ms => now;
    }
}