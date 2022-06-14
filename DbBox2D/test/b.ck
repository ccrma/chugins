// 4 balls in a square room.  Each has different bounciness (restitution).
// On each contact: 
//  - we play a note, panned to its relative room location
//  - we add impulse in random direction. 
// Notes:
//  - balls may come to rest and produce no notes.
//  - resting balls may be revived when another marauder hits them.

DbBox2D b;
int debug;
int bodies[0];
int notes[];
ModalBar inst[0];
Pan2 pans[0];
int numParticles;
1 => float sloMo;

fun void buildAudio()
{
    [57, 58, 60, 64, 65] @=> notes;
    NHHall r => dac;

    notes.size() => numParticles;
    .7 => float gain;
    inst.size(numParticles);
    pans.size(numParticles);
    for(int i;i<numParticles;i++)
    {
        ModalBar m @=> inst[i];
        m.preset(1); // Math.random2(0, 4));
        gain/numParticles => m.gain;
    
        Pan2 p @=> pans[i];
        (-1 + 2 * i/(numParticles-1.0)) => p.pan;
    
        m => p => r;
    }
}

fun void test1()
{
    bodies.size(0);
    #(0, -10) => complex gravity;
    b.worldBegin(gravity);
    // room from  x: -10, 10, y: 0, 20
    b.newRoom(#(0, 10), #(20, 20)/*sz*/, 1, b.staticType) => int room;
    bodies << room;
    for(int i;i<numParticles;i++)
    {
        complex pos;
        Math.random2f(-9, 9) => pos.re; 
        18.5 => pos.im;
        .5 => float radius;
        Math.random2f(1, 10) => float density;
        b.newCircle(pos, radius, density, b.dynamicType) => int id;
        b.setRestitution(id, Math.random2f(.75, 1.));  // energy added below
        bodies << id;
    }
    b.worldEnd();
}

buildAudio();
test1();

<<<"World has", bodies.size(), "bodies.">>>;
1::second / 60 => dur stepSize;
for(int i;i<5000;i++)
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
                    Math.random2f(.8, 1) => inst[pIndex].noteOn;
                    if(p.re < 1)
                        b.applyImpulse(id, #(Math.random2f(-.1,.1), .1)); // up random
                    else
                    if(p.re > 18)
                        b.applyImpulse(id, #(Math.random2f(-.1,.1), -.1)); // down random
                }
            }
        }
    }
}
b.getAvgSimTime() => dur simAvg;
<<<"average sim time", simAvg/1::second, "seconds, duty cycle", simAvg/stepSize>>>;

<<<"ringing">>>;
3::second => now;
