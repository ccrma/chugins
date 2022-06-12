0=> int staticType;
1 => int kinematicType;
2 => int dynamicType;

DbBox2D b;

[ 72, 74, 76, 78, 52, 55, 61, 63, 65, 66, 68, 66, 65, 63 ] @=> int notes[];
NRev r => dac;
.75 => r.gain;
.02 => r.mix;

100 => int numParticles;
ModalBar inst[numParticles];
for(int i;i<numParticles;i++)
{
    ModalBar m => inst[i];
    inst[i].preset(Math.random2(0, 4));
    inst[i] => r;
    6./numParticles => inst[i].gain;
}

int shapes[0];

fun void test1()
{
    shapes.size(0);
    #(0, -10) => complex gravity;
    b.worldBegin(gravity);
    // b.newRoom(#(0, 10), #(20, 20)/*sz*/, 1, staticType) => int room;
    b.newEdge(#(-10, 0), #(10, 0), 1/*twosided*/) => int room;
    shapes << room;
    for(int i;i<numParticles;i++)
    {
        complex pos;
        Math.random2f(-9, 9) => pos.re; 
        Math.random2f(4, 15) => pos.im;
        .5 => float radius;
        Math.random2f(.5, 1.5) => float density;
        b.newCircle(pos, radius, density, dynamicType) => int id;
        b.setRestitution(id, .95);
        // b.applyImpulse(id, #(Math.random2f(-5, 5), Math.random2f(-5, 5)));
        shapes << id;
    }
    b.worldEnd();
}

test1();

<<<"World has", shapes.size(), "shapes.">>>;
1::second / 60 => dur stepSize;
4 => float sloMo;
for(int i;i<1000;i++)
{
    b.step(stepSize);
    sloMo * stepSize => now;
    // b.pause(1);
    b.getNumContacts() => int c;
    if(c != 0)
    {
        // sonify the contacts
        for(int j;j<c;j++)
        {
            b.getContact(j) => vec3 info;
            info.y $ int -1 => int id;
            if(info.z != 0)
            {
                // Math.random2(0, notes.size()-1) => int note;
                id % notes.size() => int note;

                // <<<"press", info.x $ int, id, note >>>;
                notes[note] => Std.mtof => float f;
                f => inst[id].freq;
                Math.random2f(.7, 1) => inst[id].noteOn;
            }
            else
            {
                // <<<"release", info.x $ int, info.y $ int >>>;
                // 1 => inst[id].noteOff;
            }
        }
    }
    else
    {
        // sonify other aspects of the shapes.
        /*
        b.getPosition(shapes[1]) => complex p;
        if(p.im < 2)
        {
            <<<"shapes[1] p", p>>>;
            <<<"          v", b.getVelocity(shapes[1])>>>;
        }
        */
    }
}
b.getAvgSimTime() => dur simAvg;
<<<"average sim time", simAvg/1::second, "seconds, duty cycle", simAvg/stepSize>>>;

<<<"ringing">>>;
3::second => now;
