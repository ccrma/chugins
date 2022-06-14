// 14 bouncy balls above a plane. Each is placed randomly and
// has a random density.
//
// On each contact: 
//  - we play a note
// Notes:
//  - balls may fall off the plane nevery to be heard again.
//  - resting balls may be revived when another marauder hits them.
DbBox2D b;

[72, 74, 76, 78, 52, 55, 61, 63, 65, 66, 68, 66, 65, 63] @=> int notes[];
NRev r => dac;
.75 => r.gain;
.02 => r.mix;

notes.size() => int numParticles;
ModalBar inst[numParticles];
for(int i;i<numParticles;i++)
{
    ModalBar m @=> inst[i];
    inst[i].preset(Math.random2(0, 4));
    inst[i] => r;
    6./numParticles => inst[i].gain;
}

int bodies[0];

fun void test1()
{
    bodies.size(0);
    #(0, -10) => complex gravity;
    b.worldBegin(gravity);
    // things can fall off the edge into the abyss
    b.newEdge(#(-10, 0), #(10, 0)) => int room;
    bodies << room;
    for(int i;i<numParticles;i++)
    {
        complex pos;
        Math.random2f(-9, 9) => pos.re; 
        Math.random2f(4, 15) => pos.im;
        .5 => float radius;
        Math.random2f(.5, 1.5) => float density;
        b.newCircle(pos, radius, density, b.dynamicType) => int id;
        b.setRestitution(id, .95);
        bodies << id;
    }
    b.worldEnd();
}

test1();

<<<"World has", bodies.size(), "bodies.">>>;
1::second / 60 => dur stepSize;
4 => float sloMo;
for(int i;i<1000;i++)
{
    b.step(stepSize);
    sloMo * stepSize => now;
    b.getNumContacts() => int c;
    if(c != 0)
    {
        // sonify the contacts
        for(int j;j<c;j++)
        {
            b.getContact(j) => vec3 info; //  (bodyA, bodyB, touching)
            info.y $ int -1 => int id;
            if(info.z != 0)
            {
                id % notes.size() => int note;

                notes[note] => Std.mtof => float f;
                f => inst[id].freq;
                Math.random2f(.7, 1) => inst[id].noteOn;
            }
        }
    }
}
b.getAvgSimTime() => dur simAvg;
<<<"average sim time", simAvg/1::second, "seconds, duty cycle", simAvg/stepSize>>>;

<<<"ringing">>>;
3::second => now;
