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

## API

Because many Box2d parameters are 2 or 3 dimensional we leverage
ChucK's builtin types, `complex` and `vec3` to improve interface
expressivity.

### Scene Construction

New shapes are created between calls to `worldBegin` and `worldEnd`.
Each shape is assigned a shape id which can be used to query the current
state of various parameters.  The `bodyType` parameter is can be one of
of three values `staticType`, `kinematicType`, or `dynamicType`, available
as a member variable off the DbBox2D instance.

| Method                                                                                  | Description                                                                       |
| :-------------------------------------------------------------------------------------- | :-------------------------------------------------------------------------------- |
| `worldBegin(complex gravity)`                                                           | cancels current world and starts a new one.                                       |
| `worldEnd()`                                                                            | finishes the current world definition. Simulation can now happen.                 |
| `int newPoint(complex pos)`                                                             | creates a static point. Often used as joint anchor ponit.                    |
| `int newEdge(complex p1, complex p2)`                                                   | creates a solid, two-sided static line. Often used for floors.                    |
| `int newTriangle(complex p1, complex p2, complex p3, float density, int bodyType)`      | creates a solid triangle.                                                         |
| `int newCircle(complex pos, float radius, float density, int bodyType)`                 | creates a solid circle.                                                           |
| `int newRectangle(complex pos, complex size, float angle, float density, int bodyType)` | creates a solid rectangle.                                                        |
| `int newRoom(complex pos, complex size, float density, int bodyType)`                     | creates a "room" comprised of 4 rectangles.  Usually staticType or kinematicType. |

Joints can be added to dynamically constrain bodies in the simulation.

| Method                       | Description               |
| :--------------------------- | :------------------------ |
| `int newRevoluteJoint(int bodyA, int bodyB, complex localAnchorA, complex localAnchorB, float refAngle, float motorSpeed, float maxMotorTorque)` | creates a revolute joint. Returns a _joint_ id.|
| `int newDistanceJoint` (wip) | creates a distance joint. |
| `int newSpringJoint`   (wip) | creates a distance joint. |

### Simulation Query + Control

Once you have created a world you can control and query the simulation.
Object properties can be changed during the simulation. To modify object
properities its class (Body, Contact, Joint) couples with its id to uniquely
identify the object.

| Method                                 | Description                                                                             |
| :------------------------------------- | :-------------------------------------------------------------------------------------- |
| `step(dur am)`                         | causes the simulation to proceed to its next step. amt is typically 1/60th of a second. |
| `dur getAvgSimTime()`                  | returns the average measured compute time for a simulation step.                        |
| `complex getPosition(int id)`          | returns the position of the identified _body.                                         |
| `complex getVelocity(int id)`          | returns the linear velocity of the identified _body.                                  |
| `complex getAngularVelocity(int id)`   | returns the angular velocity of the identified _body.                                 |
| `int getNumContacts()`                 | returns the number of contact events that occured in the last frame.                    |
| `vec3 getContact(int id)`              | returns the contact info for the identified _contact_. 3 values are stuffed into the vec2: shapeAId, shapeBId, touching. |
| `setGravity(complex g)`                | allows you to change gravity during the simulation.                                     |
| `setFriction(int id, float x)`         | change the friction value for identified _body.                                       |
| `setDensity(int id, float x)`          | change the density value for identified _body.                                        |
| `setRestitution(int id, float x)`      | change the restitution (bounciness) value for identified _body.                       |
| `applyImpulse(int id, complex x)`      | apply an impulse force to the identified _body.                                       |
| `applyAngularImpulse(int id, float x)` | apply an angular impulse force to the identified _body.                               |
| `setMotorSpeed(float x)`               | change the motor speed of an existing RevoluteJoint |

### Constants

| Member                  | Description                                                            |
| :---------------------- | :--------------------------------------------------------------------- |
| `int staticType` (0)    | One of 3 body types. Used for rooms, rocks, etc.                       |
| `int kinematicType` (1) | One of 3 body types. Used for platforms. (ie: moving but "immovable"). |
| `int dynamicType` (2)   | One of 3 body types. Used for moving body. (ie: most of them).       |
| `float degToRad`        | Constant to convert from degrees to radians.                           |
| `float radToDeg`        | Constant to convert from radians to degrees.                           |

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
