# DbBox2D Chugin

## Intro

DbBox2D is a [ChucK](https://chuck.stanford.edu/) chugin built atop
the [Box2D](https://box2d.org/) physics simulation engine.  DbBox2D
provides a chuck binding to a subset of of the Box2D API for scene 
specification and simulation control.

The Big Idea is that you can use simulated physical effects to trigger 
musical events.  Another way to think of it, is that you can use ChucK to
[sonify](https://en.wikipedia.org/wiki/Sonification) your simulation. 

This chugin is included in [Fiddle](https://cannerycoders.com/#Fiddle),
the free ChucK development environment.

## test/example.ck

```chuck
// Example:
//  single "bouncing ball" atop a plane.
//  osc frequency modulated by distance from the ground.
//  modalbar strikes a note when contact is made.
DbBox2D b;

// define our world
b.worldBegin(#(0, -20));  // gravity points down
b.newEdge(#(-100, 0), #(100, 0)); // floor: p1, p2
b.newCircle(#(0, 20), .5, 2, b.dynamicType) => int circleId; // p, radius, density
b.setRestitution(circleId, .85); // relatively bouncy
b.worldEnd();

// sonification setup:
//  - sinosc frequency modulated by distance of circle from origin.
//  - modalbar rings on bounce events

100 => float ofreq;
4. => float distToFreq;
SqrOsc o => Envelop env => dac;
.03 => o.gain;
env.keyOn();

ModalBar m => NRev r => dac;
m.preset(1); // vibes

// run the simulation, 
1::second / 60 => dur stepSize; // aka 60 fps
for(int i;i<1000;i++)
{
    b.step(stepSize);
    stepSize => now;

    // modify o's freq
    b.getPosition(circleId) => complex pos; // x, y coords
    ofreq + Math.hypot(pos.re, pos.im) * distToFreq => o.freq;

    b.getNumContacts() => int c; // look for contacts
    if(c != 0)
    {
        b.getContact(0) => vec3 info; // (bodyA, bodyB, touching)
        if(info.z)
        {
            600 => m.freq;
            1 => m.noteOn; // strike
        }
    }
}

<<<"ringing">>>;
env.time(2); // falltime in seconds
env.keyOff();
3::second => now;
```
[API Reference](API.md)

## License

DbBox2D and Box2D are open-source.  Box2D is subject to its MIT license. 
DbBox2D is also MIT licensed:

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
