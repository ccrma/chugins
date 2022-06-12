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

// run the simulation, 
//  sinosc frequency modulated by distance of circle from origin.
//  modalbar to ring on bounce events

100 => float ofreq;
4. => float distToFreq;
SqrOsc o => Envelope env => dac;
.03 => o.gain;
env.keyOn();
ModalBar m => NRev r => dac;
m.preset(1); // vibes

1::second / 60 => dur stepSize; // aka 60 fps

for(int i;i<1000;i++)
{
    b.step(stepSize);
    stepSize => now;

    // modify o's freq
    b.getPosition(circleId) => complex pos; // x, y coords
    ofreq + Math.hypot(pos.re, pos.im) * distToFreq => o.freq;

    // look for contacts
    b.getNumContacts() => int c;
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