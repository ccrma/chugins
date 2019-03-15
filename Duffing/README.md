Duffing
=======

This chugin implements an oscillator based on the [Duffing
equation](https://en.wikipedia.org/wiki/Duffing_equation), a
non-linear second-order differential equation that models a weight on
a spring that's not exactly a spring:

    x'' + delta x' + alpha x + beta x^3 = gamma cos(omega t)

where the value of the unknown function x is displacement, t is time,
alpha is stiffness, beta is nonlinearity in the restoring force, gamma
is the amplitude of the driving force, delta is damping, and omega is
the angular frequency of the driving force.

The equation does not have an exact symbolic solution, so we
approximate it using RK4, the classical [Runge-Kutta
method](https://en.wikipedia.org/wiki/Runge%E2%80%93Kutta_methods). Because
RK4 operates on first-order differential equations, we decompose the
second-order equation into a system of two first-order equations and
solve them together; by setting v to x', we get:

    x' = v
    v' = gamma cos (omega t) - delta v - alpha x - beta x^3

Use
---

This chugin has getters and setters for the following coefficients:

- `alpha` aka `stiffness`
- `beta` aka `nonlinearity`
- `gamma` aka `drive`
- `delta` aka `damping`
- `omega` aka `freq` (though note that this is not the frequency of
  the oscillator)

In addition, you can get and set the initial conditions (`x` and `v`)
and perform a `reset()` to set time back to zero, which occurs
whenever you change either of the initial conditions.

Finally, you can get and set the value of `h` aka `step`; this is the
size of the interval used in the RK4 calculation, and defaults to one
over the sample rate. This is the "natural" interval, since we're
calculating at every tick, but you can change it, consequently
changing the frequency of the oscillator. Note that the chugin's
notion of time is somewhat fictional; as soon as you change `h` from
the default, "time" inside the chugin will diverge from that outside.

Future work
-----------

This chugin could be generalized to solve equations other than Duffing
and to use numerical methods other than RK4. It also could have a test
mode for comparing the output of a given method with the true values
for an equation that has a symbolic solution.

Thanks
------

The motivation for this chugin came from hearing Tom Mudd's superb
[Gutter Synthesis](https://entracte.co.uk/projects/tom-mudd-e226/),
which "uses an interrelated set of eight Duffing oscillators and
associated filter banks." I haven't examined his implementation, so
all credit goes to him for inspiration, and none for the mistakes of
this amateur mathematician.
