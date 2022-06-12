// 4 balls in a square room.  Each has different bounciness (restitution).
// On each contact: 
//  - we play a note, panned to its relative room location
//  - we add impulse in upper-right direction. 
// Notes:
//  - balls may come to rest and produce no notes.
//  - resting balls may be revived when another marauder hits them.

DbBox2D b;
int debug;

[72, 64, 46, 60] @=> int notes[];
NHHall r => dac;

1 => float sloMo;
notes.size() => int numParticles;
.7 => float gain;
ModalBar inst[numParticles];
Pan2 pans[numParticles];
for(int i;i<numParticles;i++)
{
    ModalBar m @=> inst[i];
    m.preset(1); // Math.random2(0, 4));
    gain/numParticles => m.gain;

    Pan2 p @=> pans[i];
    (-1 + 2 * i/(numParticles-1.0)) => p.pan;

    m => p => r;
}

int shapes[0];

fun void test1()
{
    shapes.size(0);
    #(0, -10) => complex gravity;
    b.worldBegin(gravity);
    // room from  x: -10, 10, y: 0, 20
    b.newRoom(#(0, 10), #(20, 20)/*sz*/, 1, b.staticType) => int room;
    shapes << room;
    for(int i;i<numParticles;i++)
    {
        complex pos;
        Math.random2f(-9, 9) => pos.re; 
        18.5 => pos.im;
        .5 => float radius;
        Math.random2f(1, 10) => float density;
        b.newCircle(pos, radius, density, b.dynamicType) => int id;
        b.setRestitution(id, Math.random2f(.75, 1.));  // energy added below
        shapes << id;
    }
    b.worldEnd();
}

test1();

<<<"World has", shapes.size(), "shapes.">>>;
1::second / 60 => dur stepSize;
for(int i;i<10000;i++)
{
    b.step(stepSize);
    sloMo * stepSize => now;
    b.getNumContacts() => int c;
    if(c != 0)
    {
        // sonify the contacts
        for(int j;j<c;j++)
        {
            b.getContact(j) => vec3 info; // (bodyA, bodyB, touching)
            if(info.z != 0)
            {
                int id;
                if(info.x $ int == 0) // 0 is room, we only play notes for the spheres
                    info.y $ int => id;
                else
                if(info.y $ int == 0)
                    info.x $ int => id;
                if(id > 0) 
                {
                    id -1 => int pIndex;
                    pIndex % notes.size() => int note;
                    b.getPosition(id) => complex p;
                    b.getVelocity(id) => complex v;
                    -1 + (p.re + 10) / 10 => pans[pIndex].pan; // x coord is on [-10, 10]
                    if(debug && id == 1)
                    {
                        <<<id, p.re, p.im, "pan", pans[pIndex].pan()>>>;
                        <<<"v", v.re, v.im>>>;
                    }
                    notes[note] => Std.mtof => float f;
                    f => inst[pIndex].freq;
                    Math.random2f(.7, 1) => inst[pIndex].noteOn;
                    b.applyImpulse(id, #(.1, .1)); // up right
                }
            }
        }
    }
}
b.getAvgSimTime() => dur simAvg;
<<<"average sim time", simAvg/1::second, "seconds, duty cycle", simAvg/stepSize>>>;

<<<"ringing">>>;
3::second => now;
