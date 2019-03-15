// This chugin implements an oscillator based on the [Duffing
// equation](https://en.wikipedia.org/wiki/Duffing_equation), a
// non-linear second-order differential equation that models a weight on
// a spring that's not exactly a spring:
//
//     x'' + delta x' + alpha x + beta x^3 = gamma cos(omega t)
//
// where the value of the unknown function x is displacement, t is time,
// alpha is stiffness, beta is nonlinearity in the restoring force, gamma
// is the amplitude of the driving force, delta is damping, and omega is
// the angular frequency of the driving force.
//
// Options:
//
// alpha aka stiffness (float): default 1.0
// beta aka nonlinearity (float): default 5.0
// gamma aka drive (float): default 8.0
// delta aka damping (float): default 0.02
// omega aka freq (float): default 0.5
//   (though note that this is not the frequency of the oscillator)
//
// x (float): initial position; default 0.0
// v (float): initial velocity; default 0.0
// reset (void): reset time to zero; called whenever you set x or v
//
// h aka step (float): interval used for calculation, default 1.0 / srate
//   changing this can change the "frequency" of the oscillator

Duffing d[3];

for (0 => int i ; i < 3 ; i++) {
  d[i] => blackhole;
}

SawOsc s => OneZero filter => Gain g => dac;

0.1 => g.gain;
220.0 => float base;

0.00001 => d[0].step;
0.0003 => d[1].step;
0.00007 => d[2].step;

100.0 => d[2].beta;

fun void update() {
  while (true) {
    d[0].last() => filter.b0;
    d[1].last() => filter.b1;
    d[2].last() * base => s.freq;
    1::samp => now;
  }
}

spork ~ update();

10::second => now;
